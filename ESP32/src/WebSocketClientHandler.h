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
#include <list>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"
#include "esp_websocket_client.h"

class WebSocketClientHandler{
    public: 
        static bool isConnected;
        static esp_websocket_client_handle_t client;
        static char init_params[256];
        void setup(const char* websockets_server_host, const int websockets_server_port, const char* device_name) {
            LogHandler::info(_TAG, "Setting up webSocket client");
            esp_websocket_client_config_t websocket_cfg = {};
            String wsUri("ws://"+String(websockets_server_host)+":"+String(websockets_server_port)+"/");
            websocket_cfg.uri = wsUri.c_str();
            String initJson("{\"identifier\":\"" + String(SettingsHandler::hostname) + "\", \"address\":\"00000000\", \"version\":0}");
            strcpy(init_params,initJson.c_str());
            if(client){
                esp_websocket_client_destroy(client);
            }
            client = esp_websocket_client_init(&websocket_cfg);
            esp_websocket_client_start(client);

            esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
        }

        void read(char* wsData){
            if(strlen(lastMessage)>0) {
                strcpy(wsData,lastMessage);
                memset(lastMessage,'\0',256);
            }
        }
        
        void closeAll()
        {
            esp_websocket_client_stop(client);
            isConnected = false;
        }

    private:
        // std::mutex serial_mtx;
        // std::mutex command_mtx;
        const char* _TAG = TagHandler::WebSocketsClientHandler;
// unsigned long lastCall;
        static int m_lastSend;
        static char lastMessage[256];

        static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
            switch (event_id) {
            case WEBSOCKET_EVENT_CONNECTED:
                LogHandler::info(TagHandler::WebSocketsClientHandler, "WEBSOCKET_EVENT_CONNECTED");
                esp_websocket_client_send_text(client, init_params, strlen(init_params), portMAX_DELAY);
                isConnected = true;
                break;
            case WEBSOCKET_EVENT_DISCONNECTED:
                LogHandler::info(TagHandler::WebSocketsClientHandler, "WEBSOCKET_EVENT_DISCONNECTED");
                isConnected = false;
                break;
            case WEBSOCKET_EVENT_DATA:
                LogHandler::debug(TagHandler::WebSocketsClientHandler, "WEBSOCKET_EVENT_DATA");
                LogHandler::debug(TagHandler::WebSocketsClientHandler, "Received opcode=%d", data->op_code);
                LogHandler::debug(TagHandler::WebSocketsClientHandler, "Received=%.*s", data->data_len, (char *)data->data_ptr);
                LogHandler::debug(TagHandler::WebSocketsClientHandler, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
                if(data->op_code==2 && data->data_len>0){
                    if(strlen(lastMessage)==0){
                        strncpy(lastMessage,(char *)data->data_ptr,data->data_len);
                    } else{
                        if(strlen(lastMessage)+data->data_len>255){
                            LogHandler::info(TagHandler::WebSocketsClientHandler, "LastMessage len:%d too long,new message is %d, skip new message", strlen(lastMessage),data->data_len);
                        } else{ 
                            lastMessage[strlen(lastMessage)]=' ';
                            strncpy(lastMessage+strlen(lastMessage),(char *)data->data_ptr,data->data_len);
                        }
                    }
                    LogHandler::debug(TagHandler::WebSocketsClientHandler, "LastMessage len:%d msg:%s", strlen(lastMessage),lastMessage);
                }
                break;
            case WEBSOCKET_EVENT_ERROR:
                LogHandler::info(TagHandler::WebSocketsClientHandler, "WEBSOCKET_EVENT_ERROR");
                break;
            }
        }

};

bool WebSocketClientHandler::isConnected = false;
char WebSocketClientHandler::lastMessage[256] = {0};
esp_websocket_client_handle_t WebSocketClientHandler::client = nullptr;
char WebSocketClientHandler::init_params[256] = {0};
