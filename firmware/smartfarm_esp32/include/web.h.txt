#pragma once
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <config.h>

extern WebServer server; 

void setupRoutes();
void handleFileRead(const String& pathReq);
void handleStatus();
void handleAct();

void writeNote(const String& s);
void handleCfgGet();
void handleCfgPost();
String contentTypeFor(String filename);