/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */


#pragma once

#include <Arduino.h>
#include "SettingsHandler.h"
#include "utils.h"
#include "TagHandler.h"
#include "../lib/struct/command.hpp"

class SystemCommandHandler {
public: 
	SystemCommandHandler() {
		tCodeQueue = xQueueCreate(10, sizeof(char[MAX_COMMAND]));
		if(tCodeQueue == NULL) {
			LogHandler::error(_TAG, "Error creating the tcode queue");
		}
	}
    bool process(const char* in) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if(isSaveCommand(in)) {
			LogHandler::debug(_TAG, "Enter process save command: %s", in);
			for(Command command : saveCommands) {
				if(match(in, command.command)) {
					command.callback();
					xSemaphoreGive(xMutex);
					return true;
				}
			}

			LogHandler::error(_TAG, "Unknown save command: %s\n", in);
			xSemaphoreGive(xMutex);
			return false;

		} else if(isOtherCommand(in)) {
			LogHandler::debug(_TAG, "Enter process other command: %s", in);

			for(auto command : commands) {
				if(match(in, command.command)) {
					command.callback();
					xSemaphoreGive(xMutex);
					return true;
				}
			}

			for(auto command : commandExternal) {
				if(startsWith(in, command.command)) {
					command.callback(in);
					xSemaphoreGive(xMutex);
					return true;
				}
			}
			
			// Commands with values
			int indexofDelim = getposition(in, strlen(in), DELEMITER_VALUE);
			if(indexofDelim == -1) {
				LogHandler::error(_TAG, "Invalid command format: '%s' missing colon, correct format is #<command>:<value>", in);
				xSemaphoreGive(xMutex);
				return false;
			}
			const char* commandAlone = substr(in, 0, indexofDelim);
			if(!strlen(commandAlone)) {
				LogHandler::error(_TAG, "Invalid command format: '%s' missing command, correct format is #<command>:<value>", in);
				xSemaphoreGive(xMutex);
				return false;
			}
			const char* value = substr(in, indexofDelim +1, strlen(in));
			if(!strlen(value)) {
				LogHandler::error(_TAG, "Invalid command format: '%s' missing value, correct format is #<command>:<value>", in);
				xSemaphoreGive(xMutex);
				return false;
			}

			LogHandler::debug(_TAG, "Value command: %s:%s", commandAlone, value);
			for(auto command : commandCharValues) {
				if(match(commandAlone, command.command)) {
					command.callback(value);
					xSemaphoreGive(xMutex);
					return true;
				}
			}

			for(auto command : commandNumberValues) {
				if(match(commandAlone, command.command)) {
					int valueInt = getInt(value);
					command.callback(valueInt);
					xSemaphoreGive(xMutex);
					return true;
				}
			}

			LogHandler::error(_TAG, "Unknown command: %s", in);
			xSemaphoreGive(xMutex);
			return false;
		}
		xSemaphoreGive(xMutex);
		return false;
	}

	// bool process(ButtonModel* button, char buf[MAX_COMMAND]) {
	// 	buf[0] = {0};
	// 	for(auto command : commandExternal) {
	// 		if(match(button->command, command.command)) {
	// 			strlcpy(buf, button->command, MAX_COMMAND);
	// 			button->isPressed() ? strcat(buf, ":1\n") : strcat(buf, ":0\n");
	// 			return true;
	// 		}
	// 	}
	// 	if(!button->isPressed()) {// Filter out other commands button release event for now.
	// 		strlcpy(buf, button->command, MAX_COMMAND);
	// 	} 
	// 	return false;
	// }

	/// @brief This function is mainly for concatenating the button state for commands sent externally.
	/// @param button 
	/// @param buf 
	/// @return true if any commands where added.
	bool process(ButtonModel* button, char buf[MAX_COMMAND]) {
		LogHandler::debug(_TAG, "Enter process button command: %s", button->command);
		char temp[MAX_COMMAND];
		strlcpy(temp, button->command, MAX_COMMAND);
   		char *token = strtok(temp, " ");//Split incoming at TCode delemiter "space"
		buf[0] = {0};
		while( token != NULL ) {// Specify if the button is pressed or released only for externaly sent commands.
			LogHandler::debug(_TAG, "Searching command: %s", token);
			bool externalFound = false;
			for(auto command : commandExternal) {
				if(match(command.command, token)) {
					strcat(buf, token);
					button->isPressed() ? strcat(buf, ":1") : strcat(buf, ":0");
					externalFound = true;
					break;
				}
			}
			if(!externalFound && !button->isPressed()) {// Add other commands only if the button has been released for now.
				strcat(buf, token);
			}
			strcat(buf, " ");
      		token = strtok(NULL, " ");
		}
		if(strlen(buf)) {
			strcat(buf, "\n");
			LogHandler::debug(_TAG, "Finish process button command: %s", buf);
			return true;
		}
		return false;
	}

	bool isCommand(const char* in) {
		return isSaveCommand(in) || isOtherCommand(in);
		//return strpbrk(DELEMITER_SAVE, in) != nullptr || strpbrk(DELEMITER, in) != nullptr;
	}
	
	bool isSaveCommand(const char* in) {
		return startsWith(in, DELEMITER_SAVE);
	}

	bool isOtherCommand(const char* in) {
		return startsWith(in, DELEMITER);
	}

	bool isValueCommand(const char* in) {
		 return contains(in, (&DELEMITER_VALUE));
	}
	
	void registerExternalCommandCallback(std::function<void(const char*)> callback) {
		m_externalCommandCallback = callback;
	}

    bool getTCode(char* buf) 
    {
        if(!tCodeQueue) {
            return false;
        } 
        if(!xQueueReceive(tCodeQueue, buf, 0)) {
            buf[0] = {0};
			return false;
        }
		return true;
    }

private: 
	SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
    QueueHandle_t tCodeQueue;
	const char* _TAG = TagHandler::SystemCommandHandler;
	std::function<void(const char*)> m_externalCommandCallback = 0;
	std::function<void(const char*)> m_otherCommandCallback = 0;

    const char* DELEMITER = "#";
    const char* DELEMITER_SAVE = "$"; 
    const char DELEMITER_VALUE = ':'; 
    const Command HELP{{"Help", "#help", "Print the help screen", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			printCommandHelp();
			return true;
		});
	}};
    const Command SAVE{{"Save", "$save", "Saves all settings", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			SettingsHandler::saveAll();
			Serial.println("Settings saved!");
			return true;
		});
	}};
    const Command DEFAULT_ALL{{"Default all", "$defaultAll", "Saves all settings to default", SaveRequired::NO, RestartRequired::YES, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			SettingsHandler::defaultAll();
			Serial.println("All settings reset to default!");
			return true;
		}, SaveRequired::NO, RestartRequired::YES);
	}};
    const Command RESTART{{"Restart", "#restart", "Restart the system", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([]() -> bool {
			SettingsHandler::restart();
			return true;
		});
	}};
    const Command CLEAR_LOGS_INCLUDE{{"Clear log include", "#clear-log-include", "Clears all the log included tags", SaveRequired::YES, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			LogHandler::clearIncludes();
			Serial.println("Tags cleared");
			return true;
		}, SaveRequired::YES);
	}};
    const Command CLEAR_LOGS_EXCLUDE{{"Clear log exclude", "#clear-log-exclude", "Clears all the log excluded tags", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			LogHandler::clearExcludes();
			Serial.println("Tags filters cleared");
			return true;
		}, SaveRequired::NO);
	}};
    const Command MOTION_ENABLE{{"Motion enable", "#motion-enable", "Enables the motion generator", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return validateBool("Motion", true, SettingsHandler::getMotionEnabled(), [](bool value) -> bool {
			SettingsHandler::setMotionEnabled(value);
			return true;
		});
	}};
    const Command MOTION_DISABLE{{"Motion disable", "#motion-disable", "Disables the motion generator", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return validateBool("Motion", false, SettingsHandler::getMotionEnabled(), [](bool value) -> bool {
			SettingsHandler::setMotionEnabled(value);
			return true;
		});
	}};
    const Command MOTION_TOGGLE{{"Motion toggle", "#motion-toggle", "Toggles the motion generator", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			SettingsHandler::setMotionEnabled(!SettingsHandler::getMotionEnabled());
			Serial.println(SettingsHandler::getMotionEnabled() ? "Motion enabled" : "Motion disabled");
			return true;
		});
	}};
    const Command MOTION_HOME{{"Motion home", "#device-home", "Sends all axis' to its home position", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		char buf[MAX_COMMAND];
		SettingsHandler::channelMap.tCodeHome(buf);
		LogHandler::debug(_TAG, "Device home: %s", buf);
		writeTCode(buf);
		return true;
	}};
	const Command WEBSOCKET_CONNECT{{"websocket connect", "#websocket-connect", "Connect to remote websocket server", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
			SettingsHandler::wsClientReconnect = true;
			return true;
	}};

    const Command MOTION_PROFILE_CYCLE{{"Motion profile cycle", "#motion-profile-cycle", "Cycles the motion generator profiles stopping after last profile", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this]() -> bool {
		return execute([this]() -> bool {
			SettingsHandler::cycleMotionProfile();
			return true;
		});
	}};
    const Command PAUSE{{"Pause", "#pause", "Pauses all motion of the device", SaveRequired::YES, RestartRequired::YES, CommandValueType::NONE}, [this]() -> bool {	
		return execute([this]() -> bool {
			SettingsHandler::motionPaused = true;
			LogHandler::debug(_TAG, "Device paused");
			return true;
		});
		return true;
	}};
    const Command RESUME{{"Resume", "#resume", "Resumes all motion of the device", SaveRequired::YES, RestartRequired::YES, CommandValueType::NONE}, [this]() -> bool {	
		return execute([this]() -> bool {
			SettingsHandler::motionPaused = false;
			LogHandler::debug(_TAG, "Device resumed");
			return true;
		});
		return true;
	}};
    const Command PAUSE_TOGGLE{{"Pause toggle", "#pause-toggle", "Pauses all motion of the device", SaveRequired::YES, RestartRequired::YES, CommandValueType::NONE}, [this]() -> bool {	
		return execute([this]() -> bool {
			SettingsHandler::motionPaused = !SettingsHandler::motionPaused;
			LogHandler::debug(_TAG, SettingsHandler::motionPaused ? "Device paused" : "Device resumed");
			return true;
		});
		return true;
	}};
    const CommandValue<const int> MOTION_HOME_SPEED{{"Motion home", "#device-home", "Sends all axis' to its home position at specified speed (S)", SaveRequired::NO, RestartRequired::NO, CommandValueType::NUMBER}, [this](const int value) -> bool {
		char buf[MAX_COMMAND];
		SettingsHandler::channelMap.tCodeHome(buf, value);
		LogHandler::debug(_TAG, "Device home speed: %s", buf);
		writeTCode(buf);
		return true;
	}};
    const CommandValue<const char*>WIFI_SSID{{"Wifi ssid", "#wifi-ssid", "Sets the ssid of the wifi AP", SaveRequired::YES, RestartRequired::YES, CommandValueType::STRING}, [this](const char* value) -> bool {
		return validateMaxLength("Wifi SSID", value, sizeof(SettingsHandler::ssid), false, [](const char* value) -> bool {
			strcpy(SettingsHandler::ssid, value);
			return true;
		}, SaveRequired::YES, RestartRequired::YES); 
	}};
    const CommandValue<const char*>WIFI_PASS{{"Wifi pass", "#wifi-pass", "Sets the password of the wifi AP", SaveRequired::YES, RestartRequired::YES, CommandValueType::STRING}, [this](const char* value) -> bool {
		return validateMaxLength("Wifi password", value, sizeof(SettingsHandler::wifiPass), true, [](const char* value) -> bool {
			strcpy(SettingsHandler::wifiPass, value);
			return true;
		}, SaveRequired::YES, RestartRequired::YES); 
	}};
    const CommandValue<const int>LOG_LEVEL{{"Log level", "#log-level", "Sets system log level", SaveRequired::YES, RestartRequired::NO, CommandValueType::NUMBER}, [this](const int value) -> bool {
		return executeValue<const int>(value, [this](const int value) -> bool {
			if(value > (int)LogLevel::VERBOSE) {
				LogHandler::error(_TAG, "Invalid value: %ld. Valid log levels are 0-4", value);
				return false;
			}
			SettingsHandler::logLevel = static_cast<LogLevel>(value);
			LogHandler::setLogLevel(static_cast<LogLevel>(value));
			Serial.printf("Log level changed to: %ld\n", value);
			return true;
		}, SaveRequired::YES);
	}};
    const CommandValue<const char*>ADD_LOG_INCLUDE{{"Add log include", "#add-log-include", "Adds a tag to the log includes", SaveRequired::YES, RestartRequired::NO, CommandValueType::STRING}, [this](const char* value) -> bool {
		return executeValue<const char*>(value, [](const char* value) -> bool {
			if(!LogHandler::addInclude(value)) {
			Serial.printf("Tag already exists: %s\n", value);
				return false;
			}
			Serial.printf("Log level tag added: %s\n", value);
			return true;
		}, SaveRequired::YES);
	}};
    const CommandValue<const char*>REMOVE_LOG_INCLUDE{{"Remove log include", "#remove-log-include", "Removes a tag from the log includes", SaveRequired::YES, RestartRequired::NO, CommandValueType::STRING}, [this](const char* value) -> bool {
		return executeValue<const char*>(value, [](const char* value) -> bool {
			if(!LogHandler::removeInclude(value)) {
			Serial.printf("Tag did not exist: %s\n", value);
				return false;
			}
			Serial.printf("Log level tag removed: %s\n", value);
			return true;
		}, SaveRequired::YES);
	}};
    const CommandValue<const char*>ADD_LOG_EXCLUDE{{"Add log exclude", "#add-log-exclude", "Adds a tag to the log excludes", SaveRequired::YES, RestartRequired::NO, CommandValueType::STRING}, [this](const char* value) -> bool {
		return executeValue<const char*>(value, [](const char* value) -> bool {
			if(!LogHandler::addExclude(value)) {
			Serial.printf("Tag filter already exists: %s\n", value);
				return false;
			}
			Serial.printf("Log level tag filter added: %s\n", value);
			return true;
		}, SaveRequired::YES);
	}};
    const CommandValue<const char*>REMOVE_LOG_EXCLUDE{{"Remove log exclude", "#remove-log-exclude", "Removes a tag from the log excludes", SaveRequired::YES, RestartRequired::NO, CommandValueType::STRING}, [this](const char* value) -> bool {
		return executeValue<const char*>(value, [](const char* value) -> bool {
			if(!LogHandler::removeExclude(value)) {
				Serial.printf("Tag filter did not exist: %s\n", value);
				return false;
			}
			Serial.printf("Log level tag filter removed: %s\n", value);
			return true;
		}, SaveRequired::YES);
	}};
    const CommandValue<const char*>MOTION_PROFILE_NAME{{"Motion profile set by name", "#motion-profile-name", "Sets the current running profile by name", SaveRequired::NO, RestartRequired::NO, CommandValueType::STRING}, [this](const char* value) -> bool {
		return validateMaxLength("Motion profile name", value, maxMotionProfileNameLength, false, [](const char* value) -> bool {
			SettingsHandler::setMotionProfileName(value);
			return true;
		});
	}};
    const CommandValue<const int>MOTION_PROFILE_SET{{"Motion profile set by number", "#motion-profile-set", "Sets the current running profile by number", SaveRequired::NO, RestartRequired::NO, CommandValueType::NUMBER}, [this](const int value) -> bool {
		return validateGreaterThanZero("Motion profile", value, [this](int value) -> bool {
			int profileAsIndex = value - 1;
			if(profileAsIndex > maxMotionProfileCount) {
				LogHandler::error(_TAG, "Motion profile %ld does not exist", profileAsIndex);
				return false;
			}
			SettingsHandler::setMotionProfile(profileAsIndex);
			return true;
		});
	}};
    const CommandValue<const char*> EDGE{{"Edge", "#edge", "Outputs the edge pressed command to external application", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this](const char* in) -> bool {
		if(m_externalCommandCallback) {
			m_externalCommandCallback(in);
		}
		return true;
	}};
    const CommandValue<const char*> LEFT{{"Left", "#left", "Outputs the left pressed command to external application", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this](const char* in) -> bool {
		if(m_externalCommandCallback) {
			m_externalCommandCallback(in);
		}
		return true;
	}};
    const CommandValue<const char*> RIGHT{{"Right", "#right", "Outputs the right pressed command to external application", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this](const char* in) -> bool {
		if(m_externalCommandCallback) {
			m_externalCommandCallback(in);
		}
		return true;
	}};
    const CommandValue<const char*> OK{{"Ok", "#ok", "Outputs the ok pressed command to external application", SaveRequired::NO, RestartRequired::NO, CommandValueType::NONE}, [this](const char* in) -> bool {
		if(m_externalCommandCallback) {
			m_externalCommandCallback(in);
		}
		return true;
	}};

	Command saveCommands[2] {
        SAVE,
        DEFAULT_ALL,
	};

    Command commands[13] = {
        HELP,
        RESTART,
        CLEAR_LOGS_INCLUDE,
        CLEAR_LOGS_EXCLUDE,
        MOTION_ENABLE,
        MOTION_DISABLE,
        MOTION_TOGGLE,
        MOTION_PROFILE_CYCLE,
		WEBSOCKET_CONNECT,
		PAUSE,
		RESUME,
        PAUSE_TOGGLE,
		MOTION_HOME
    };

    CommandValue<const int> commandNumberValues[3] = {
        LOG_LEVEL,
        MOTION_PROFILE_SET,
		MOTION_HOME_SPEED
    };

    CommandValue<const char*> commandCharValues[7] = {
        WIFI_SSID,
        WIFI_PASS,
        ADD_LOG_INCLUDE,
        REMOVE_LOG_INCLUDE,
        ADD_LOG_EXCLUDE,
        REMOVE_LOG_EXCLUDE,
        MOTION_PROFILE_NAME
    };
    CommandValue<const char*> commandExternal[4] = {
        EDGE,
        LEFT,
        RIGHT,
        OK
    };

	void writeTCode(char tcode[MAX_COMMAND]) {
		if(tCodeQueue)
        	xQueueSend(tCodeQueue, tcode, 0);
	}

	int getInt(const char* value) {
		return (int)(String(value).toInt());
	}

	bool execute(std::function<bool()> function, SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		bool subValidate = function();
		if(subValidate) {
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	template<class T>
	bool executeValue(T value, std::function<bool(T)> function,  SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		bool subValidate = function(value);
		if(subValidate) {
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	bool validateBool(const char* name, bool value, bool currentValue, std::function<bool(bool value)> function,  SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		if(value && currentValue) {
			Serial.println("Already on!");
			xSemaphoreGive(xMutex);
			return false;
		}
		if(!value && !currentValue) {
			Serial.println("Already off!");
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			printNewState(name, value);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	bool validateGreaterThanZero(const char* name, int value, std::function<bool(int value)> function,  SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		if(value < 1) {
			Serial.printf("Invalid value: %ld.", value);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			printNewState(name, value);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}
	bool validateGreaterThanNegativeOne(const char* name, int value, std::function<bool(int value)> function, SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		if(value < 0) {
			Serial.printf("Invalid value: %ld.", value);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			printNewState(name, value);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	bool validateMaxLength(const char* name, const char* value, int maxLen, bool valueSensitive, std::function<bool(const char* value)> function,  SaveRequired isSaveRequired = SaveRequired::NO, RestartRequired isRestartRequired = RestartRequired::NO) {
		if(strlen(value) > maxLen) {
			Serial.printf("Invalid command: %s max length is: %ld\n", name, maxLen);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			if(!valueSensitive)
				printNewState(name, value);
			else
				Serial.printf("%s changed to a value of %ld length\n", name, strlen(value));
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	void printNewState(const char* name, const char* newValue) {
		Serial.printf("%s changed to: %s\n", name, newValue);
	}
	void printNewState(const char* name, int newValue) {
		Serial.printf("%s changed to: %ld\n", name, newValue);
	}
	void printNewState(const char* name, bool newValue) {
		Serial.printf("%s %s\n", name, newValue ? "enabled" : "disabled");
	}
	void printNewState(const char* name, float newValue) {
		Serial.printf("%s changed to: %f\n", name, newValue);
	}
	void printComplete(RestartRequired isRestartRequired, SaveRequired isSaveRequired) {
		if((int)isSaveRequired)
			Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
		if((int)isRestartRequired)
			Serial.println("Restart is required after save");
	}
	void printCommandHelp() {
		char buf[2500];
		printCommandHelp(buf);
		Serial.println(buf);
	}
	void printCommandHelp(char* buf) {
		// Serial.println("");
		// Serial.println("");
		// Serial.println("");
		// Serial.println("");
		buf[0] = '\0';
		strcat(buf, "\n");
		strcat(buf, "\n");
		strcat(buf, "\n");
		strcat(buf, "\n");
		strcat(buf, "Available commands:\n");
		for(Command command : saveCommands) {
			formatCommand(command, buf);
		}

		// Serial.println("#help ------------------------- Print this.");
		// Serial.println("$save ------------------------- Flush ALL settings to disk.");
		// Serial.println("$defaultAll ------------------- Reset ALL settings to default");
		// Serial.println("#restart ---------------------- Restart the esp");
		strcat(buf, "\n");
		for(Command command : commands) {
			formatCommand(command, buf);
		}

		for(auto command : commandExternal) {
			formatCommand(command, buf);
		}
		// Serial.println("Wifi:");
		// Serial.println("#wifi-ssid:value -------------- Change the wifi ssid.");
		// Serial.println("#wifi-pass:value -------------- Change the wifi password.");
		for(auto command : commandCharValues) {
			formatCommand(command, buf);
		}
		for(auto command : commandNumberValues) {
			formatCommand(command, buf);
		}
		// Serial.println("Log:");
		// Serial.println("#log-level:value -------------- Change the log level.");
		// Serial.println("    Log level values: ");
		// Serial.println("         0 -- error");
		// Serial.println("         1 -- info");
		// Serial.println("         2 -- warning");
		// Serial.println("         3 -- debug");
		// Serial.println("         4 -- verbose");
		// Serial.println("#add-log-include:value --------- Add a log tag to include");
		// Serial.println("#remove-log-include:value ------ Remove a log tag to include");
		// Serial.println("#clear-log-include ------------- Clear all included log tags");
		// Serial.println("#add-log-exclude:value --------- Add a log tag to exclude");
		// Serial.println("#remove-log-exclude:value ------ Remove a log tag to exclude");
		// Serial.println("#clear-log-exclude ------------- Clear all excluded log tags");
		// Serial.println("");
		// Serial.println("Motion generator:");
		// Serial.println("#motion-enable ----------------- Enable motion generator");
		// Serial.println("#motion-disable ---------------- Disable motion generator");
		// Serial.println("#motion-profile-set:value ------ Set the current profile");
		// Serial.printf("    Motion profile values: 1-%ld\n", maxMotionProfileCount);
		// Serial.println("#motion-toggle ----------------- Toggle motion generator");
		// Serial.println("#motion-profile-cycle ---------- Toggle and cycle through the motion profiles");
		// Serial.println("#motion-period-random-on ------- Period random on for the current profile");
		// Serial.println("#motion-period-random-off ------ Period random off for the current profile");
		// Serial.println("#motion-amplitude-random-on ---- Amplitude random on for the current profile");
		// Serial.println("#motion-amplitude-random-off --- Amplitude random off for the current profile");
		// Serial.println("#motion-offset-random-on ------- Offset random on for the current profile");
		// Serial.println("#motion-offset-random-off ------ Offset random off for the current profile");
		// Serial.println("#motion-period:value ----------- Set period for the current profile");
		// Serial.println("#motion-update:value ----------- Set update rate for the current profile");
		// Serial.println("#motion-amplitude:value -------- Set amplitude for the current profile");
		// Serial.println("#motion-offset:value ----------- Set offset for the current profile");
		// Serial.println("#motion-phase:value ------------ Set phase for the current profile");
		// Serial.println("#motion-reverse:value ---------- Set reverse for the current profile");
	}
	void formatCommand(CommandBase command, char* buf) {
		char temp[MAX_COMMAND];
		sprintf(temp, "%s%s", command.command, command.valueType == CommandValueType::NONE ? "" : command.valueType == CommandValueType::NUMBER ? ":<int>" : ":<string>");
		sprintf(temp, "%-40s", temp);
    	std::replace(temp, temp + strlen(temp), ' ', '-');
		strcat(buf, temp);
		sprintf(temp, "%s\n", command.description);
		strcat(buf, temp);
	}
};