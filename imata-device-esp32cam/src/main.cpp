/**
 * iMata ESP32-CAM Device Code
 */

#include <vector>
#include <algorithm>
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// Define the camera model
#define CAMERA_MODEL_AI_THINKER

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Another pin definition
#define FLASH_LED_GPIO_NUM 4

// Interval between photo captures in seconds.
// Can be overridden by the configuration JSON file and/or through HTTP REST API.
int CAPTURE_INTERVAL = 5;

// Maximum number of photos to capture in a session.
// Can be overridden by the configuration JSON file and/or through HTTP REST API.
int MAX_PHOTOS_PER_SESSION = 1500;

// WiFi credentials
const char *ssid = "iMata";
const char *password = "12345678";

// Global variables
Ticker photoCaptureTicker;
bool captureInProgress = false;
int photoCaptureCount = 0;
String sessionFolderName;
// Define a flag to track the request status
volatile bool isPhotoDownloadInProgress = false;

// The HTTP Web Server
AsyncWebServer server(80);

// Function to initialize the camera
bool initCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Set the frame size to the highest possible
    config.frame_size = FRAMESIZE_UXGA; // UXGA (1600x1200) resolution
    config.jpeg_quality = 10;           // Higher quality setting (lower number means higher quality)
    config.fb_count = 1;                // Reduce frame buffer count to free up memory

    // Init the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    return true;
}

// Function to initialize the MicroSD card
bool initSDCard()
{
    if (!SD_MMC.begin("/sdcard", true))
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }
    return true;
}

// Function to create a random folder name
String generateRandomFolderName()
{
    int randomNumber = random(0, 10000); // Generate a random number between 0 and 9999
    char fullPath[32];
    snprintf(fullPath, sizeof(fullPath), "/iMata-capture-%04d", randomNumber); // Format the number with leading zeros

    // Check if the folder exists, create it if not
    if (!SD_MMC.exists(fullPath))
    {
        SD_MMC.mkdir(fullPath);
    }

    return String(fullPath);
}

// Function to capture and save a photo
void capturePhoto(String targetFullPath)
{
    // Turn on flash LED
    digitalWrite(FLASH_LED_GPIO_NUM, HIGH);

    File photoFile = SD_MMC.open(targetFullPath, FILE_WRITE);
    camera_fb_t *fb = NULL;
    // Take a photo and save it to SD card
    fb = esp_camera_fb_get();
    if (!fb)
    {
        digitalWrite(FLASH_LED_GPIO_NUM, LOW);
        Serial.println("Camera capture failed");
        return;
    }
    photoFile.write(fb->buf, fb->len); // Write the image data to file
    photoFile.close();
    esp_camera_fb_return(fb);

    // Turn off flash LED
    digitalWrite(FLASH_LED_GPIO_NUM, LOW);

    Serial.println("Photo captured and saved: " + targetFullPath);
}

void addFilenameToListFile(const String &listFileFullpath, const String &photoFilename)
{
    File theListFile = SD_MMC.open(listFileFullpath, FILE_APPEND);
    if (theListFile)
    {
        theListFile.println(photoFilename);
        theListFile.close();
    }

    Serial.println("Photo filename added into list file: " + photoFilename);
}

// Function to capture photos
void capturePhotos()
{
    if (captureInProgress)
    {
        // Build the full path for the target JPEG file
        char photoFilename[21];
        sprintf(photoFilename, "iMata-photo-%04d.jpg", photoCaptureCount);

        char photoFullPath[sessionFolderName.length() + 21];
        sprintf(photoFullPath, "%s/%s", sessionFolderName.c_str(), photoFilename);

        // Capture the photo (assuming capturePhoto is a function that captures and saves the photo)
        capturePhoto(photoFullPath);
        photoCaptureCount++;

        // Save the new photo filename into related list file
        addFilenameToListFile(sessionFolderName + "/photo-list.txt", photoFilename);

        if (photoCaptureCount >= MAX_PHOTOS_PER_SESSION)
        { // Stop after capturing MAX_PHOTOS_PER_SESSION photos
            captureInProgress = false;
            photoCaptureTicker.detach();
            Serial.println("Photo capture complete");
        }
    }
}

void deleteFolder(const String &folderPath)
{
    File root = SD_MMC.open(folderPath);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        String filePath = String(folderPath) + "/" + file.name();
        if (file.isDirectory())
        {
            deleteFolder(filePath);
        }
        else
        {
            SD_MMC.remove(filePath);
        }
        file = root.openNextFile();
    }
    SD_MMC.rmdir(folderPath);
}

void deleteAllFoldersAndFiles()
{
    File root = SD_MMC.open("/");
    if (!root)
    {
        Serial.println("Failed to open root directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Root is not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        String filePath = String("/") + file.name();
        if (file.isDirectory())
        {
            deleteFolder(filePath);
        }
        else
        {
            SD_MMC.remove(filePath);
        }
        file = root.openNextFile();
    }
}

// Function to load configuration from JSON file
void loadConfigFile()
{
    File configFile = SD_MMC.open("/imata-config.json");
    if (!configFile)
    {
        Serial.println("Failed to open config file");
        return;
    }

    // Allocate a buffer to store contents of the file
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);

    // Read file to buffer
    configFile.readBytes(buf.get(), size);
    configFile.close();

    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
        Serial.println("Failed to parse config JSON file");
        return;
    }

    // Assign values to global variables
    CAPTURE_INTERVAL = doc["captureInterval"];
    MAX_PHOTOS_PER_SESSION = doc["maxPhotosPerSession"];
}

// Function to save configuration to JSON file
void saveConfigFile()
{
    DynamicJsonDocument doc(1024);
    doc["captureInterval"] = CAPTURE_INTERVAL;
    doc["maxPhotosPerSession"] = MAX_PHOTOS_PER_SESSION;

    File configFile = SD_MMC.open("/imata-config.json", FILE_WRITE);
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return;
    }

    // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0)
    {
        Serial.println("Failed to write to config file");
    }
    configFile.close();
}

// List all folders on the SD card
void handleGetSessions(AsyncWebServerRequest *request)
{
    std::vector<String> directories;

    String path = "/";
    File root = SD_MMC.open(path);
    if (!root)
    {
        request->send(500, "application/json", "{\"message\": \"Failed to open directory\"}");
        return;
    }
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            String dirName = String(file.name());

            if (dirName.indexOf("iMata-capture") != -1)
            {
                directories.push_back(dirName);
            }
        }
        file = root.openNextFile();
    }

    // Reverse the order of directories
    std::sort(directories.begin(), directories.end());

    // Create a JSON array
    DynamicJsonDocument doc(1024);
    JsonArray jsonArray = doc.to<JsonArray>();
    for (const auto &dir : directories)
    {
        jsonArray.add(dir);
    }

    // Convert JSON array to string
    String output;
    serializeJson(jsonArray, output);

    request->send(200, "application/json", output);
}

void handleDeleteSessions(AsyncWebServerRequest *request)
{
    // Delete all folders and files in SD card root directory
    deleteAllFoldersAndFiles();
    request->send(200, "application/json", "{\"message\": \"All folders deleted successfully.\"}");
}

// List JPEG files in a specific folder on the SD card
void handleGetSession(AsyncWebServerRequest *request)
{
    if (request->hasParam("folder"))
    {
        String folderName = request->getParam("folder")->value();
        String path = "/" + folderName;
        String output = "[";
        File folder = SD_MMC.open(path);
        if (!folder)
        {
            request->send(404, "application/json", "{\"message\": \"Folder not found\"}");
            return;
        }

        // First check is the file-list.txt file exists in the folder.
        // If so, just return the list of photo name in the file.
        File listFile = SD_MMC.open(path + "/photo-list.txt");
        if (listFile)
        {
            Serial.println("photo-list.txt file exists");

            while (listFile.available())
            {
                String photoFilename = listFile.readStringUntil('\n');
                photoFilename.trim();

                if (output != "[")
                    output += ',';
                output += '"' + photoFilename + '"';

                Serial.println("photoFilename: " + photoFilename);
            }
            output += "]";

            Serial.println("photo-list.txt file read done");
            listFile.close();

            request->send(200, "application/json", output);
            return;
        }

        // If the file-list.txt file does not exist, list all jpg files in the folder
        File file = folder.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                // Make sure the file has jpg extension before adding to the list
                if (String(file.name()).endsWith(".jpg"))
                {
                    if (output != "[")
                        output += ',';
                    output += '"' + String(file.name()) + '"';
                }
            }
            file = folder.openNextFile();
        }
        output += "]";
        request->send(200, "application/json", output);
    }
    else
    {
        request->send(400, "application/json", "{\"message\": \"Folder name is required\"}");
    }
}

// Handle download request for a JPEG file.
// ESP32 is a resource-limited device, so we only allow 1 concurrent photo download at a time.
void handlePhotoDownload(AsyncWebServerRequest *request)
{
    Serial.println("handlePhotoDownload called");

    // Check if a download is already in progress
    if (isPhotoDownloadInProgress)
    {
        request->send(429, "text/plain", "Too Many Requests: Only one download allowed at a time.");
        return;
    }

    // Set the flag to indicate a download is in progress
    isPhotoDownloadInProgress = true;

    if (request->hasParam("name"))
    {
        String fileName = request->getParam("name")->value();
        Serial.println("fileName: " + fileName);
        File file = SD_MMC.open("/" + fileName, FILE_READ);
        if (file)
        {
            AsyncWebServerResponse *response = request->beginResponse(
                "image/jpeg",
                file.size(),
                [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t
                {
                    if (file.available())
                    {
                        return file.read(buffer, maxLen);
                    }
                    file.close();
                    return 0;
                });
            response->addHeader("Content-Disposition", "attachment; filename=" + fileName);
            request->send(response);
        }
        else
        {
            request->send(404, "application/json", "{\"message\": \"File not found\"}");
        }
    }
    else
    {
        request->send(400, "application/json", "{\"message\": \"File name is required\"}");
    }

    // Clear the flag to indicate the download is complete
    isPhotoDownloadInProgress = false;
}

void handleDeleteSession(AsyncWebServerRequest *request)
{
    if (request->hasParam("folderPath"))
    {
        String folderPath = request->getParam("folderPath")->value();
        if (SD_MMC.exists(folderPath))
        {
            // Delete folder and all files inside
            deleteFolder(folderPath);
            request->send(200, "application/json", "{\"message\": \"Folder deleted successfully.\"}");
        }
        else
        {
            request->send(404, "application/json", "{\"message\": \"Folder not found.\"}");
        }
    }
    else
    {
        request->send(400, "application/json", "{\"message\": \"Missing folderPath parameter.\"}");
    }
}

void handleGetConfig(AsyncWebServerRequest *request)
{
    // Return a JSON object with the current configuration
    String output = "{\"captureInterval\":" + String(CAPTURE_INTERVAL) + ",\"maxPhotosPerSession\":" + String(MAX_PHOTOS_PER_SESSION) + "}";
    request->send(200, "application/json", output);
}

void handleSetConfig(AsyncWebServerRequest *request)
{
    if (request->hasParam("captureInterval") && request->hasParam("maxPhotosPerSession"))
    {
        CAPTURE_INTERVAL = request->getParam("captureInterval")->value().toInt();
        MAX_PHOTOS_PER_SESSION = request->getParam("maxPhotosPerSession")->value().toInt();
        saveConfigFile();
        request->send(200, "application/json", "{\"message\": \"Configuration updated successfully.\"}");
    }
    else
    {
        request->send(400, "application/json", "{\"message\": \"Missing parameters.\"}");
    }
}

void handleStartStopCapture(AsyncWebServerRequest *request)
{
    Serial.println("the handleStartStopCapture called");
    if (captureInProgress)
    {
        Serial.println("capture in progress, so we need to stop");
        // Stop the ticker
        photoCaptureTicker.detach();
        captureInProgress = false;
        Serial.println("Photo capture stopped.");
        request->send(200, "application/json", "{\"message\": \"Photo capture stopped.\"}");
    }
    else
    {
        Serial.println("capture is not in progress, so we need to start");
        // Start the ticker
        photoCaptureCount = 0;
        Serial.println("set photoCaptureCount done");
        captureInProgress = true;
        Serial.println("set captureInProgress done");
        sessionFolderName = generateRandomFolderName();
        Serial.println("generateRandomFolderName");
        photoCaptureTicker.attach(CAPTURE_INTERVAL, capturePhotos);
        Serial.println("photoCaptureTicker attach");
        Serial.println("Photo capture started.");

        // Create photo-list.txt empty file in the sessionFolderName
        File listFile = SD_MMC.open(sessionFolderName + "/photo-list.txt", FILE_WRITE);
        listFile.close();

        request->send(200, "application/json", "{\"message\": \"Photo capture started.\"}");
    }
}

void handleGetCaptureStatus(AsyncWebServerRequest *request)
{
    Serial.println("handleGetCaptureStatus called");
    String output = captureInProgress ? "{\"message\": \"Capture in progress.\", \"photoCaptureCount\": " + String(photoCaptureCount) + ", \"sessionFolderName\": \"" + sessionFolderName + "\"}" : "{\"message\": \"No capture in progress.\"}";
    request->send(200, "application/json", output);
}

void setup()
{
    Serial.begin(115200);

    // Initialize the flash LED pin as an output
    pinMode(FLASH_LED_GPIO_NUM, OUTPUT);

    // Power up the camera
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);

    // Initialize the camera
    if (!initCamera())
    {
        Serial.println("Camera init failed");
        return;
    }

    // Initialize the SD card
    if (!initSDCard())
    {
        Serial.println("SD Card init failed");
        return;
    }

    // Load custom config from JSON file in MicroSD card
    // loadConfigFile();

    // Start the WiFi access point
    WiFi.softAP(ssid, password);
    Serial.println("WiFi AP started");

    // Define REST API routing
    server.on("/sessions", HTTP_GET, handleGetSessions);
    server.on("/sessions", HTTP_DELETE, handleDeleteSessions);

    server.on("/session", HTTP_GET, handleGetSession);
    server.on("/photo-download", HTTP_GET, handlePhotoDownload);
    server.on("/session", HTTP_DELETE, handleDeleteSession);

    server.on("/config", HTTP_GET, handleGetConfig);
    server.on("/config", HTTP_POST, handleSetConfig);

    server.on("/start-stop-capture", HTTP_POST, handleStartStopCapture);
    server.on("/capture-status", HTTP_GET, handleGetCaptureStatus);

    // Start the HTTP server
    server.begin();
    Serial.println("HTTP server started");
}

void loop()
{
    // Nothing to do here
}
