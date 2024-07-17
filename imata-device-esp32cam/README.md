# iMata device - ESP32-Cam

## Overview

The `imata-device-esp32cam` folder contains the source code for an ESP32-CAM based device designed by AI-Thinker. The device captures photos in JPEG format using the OV2640 camera module, saves the photos to a MicroSD card, and provides an HTTP REST API for accessing these photos and start/stop the camera capturing task. The project is written in C++ and uses the PlatformIO embedded development suite for development and deployment.

## Features

1. Captures photos in JPEG format using the OV2640 camera module.
1. Saves captured photos to a MicroSD card.
1. Provides an HTTP REST API using ESPAsyncWebServer to list and download photos.
1. Compatible with the iMata React Native app for start/stop capturing photos and displaying images.
1. Can be powered using Lithium batteries or a power bank.

## How It Works

- `Photo Capture`: The device captures photos using the OV2640 camera module.
- `Saving Photos`: The captured photos are saved in JPEG format to a MicroSD card.
- `HTTP REST API`: The device runs an HTTP REST API server using ESPAsyncWebServer. The API allows the iMata React Native app to list and download JPEG photos stored on the MicroSD card, also start/stop photo capure task.
- `Power Supply`: The ESP32-CAM can be powered using Lithium batteries or a power bank, providing flexibility for various use cases and environments.

## Getting Started

### Prerequisites

- PlatformIO installed on your development machine.
- ESP32-CAM compatible board by AI-Thinker.
- OV2640 camera module.
- MicroSD card.
- Power source (Lithium batteries or power bank).

### Installation

Clone the repository:

```
git clone https://github.com/andypmw/imata.git
```

Then open the project on folder `imata-device-esp32cam` with PlatformIO.

### Configuration

Update the configuration files with your specific settings, such as WiFi credentials and other parameters needed for your environment.

### Build and Upload

Connect your ESP32-CAM board to your development machine.

Build and upload the firmware:

```
pio run --target upload
```

### Exposed API Endpoints by iMata ESP32-Cam HTTP Server

- `GET /sessions` to get list of captured sessions
- `GET /session?folder=<SessionName>` to get list of photo paths on a captured session
- `GET /photo-download?name=<PhotoName>` to download the JPEG file
- `GET /capture-status` to check is the device actively capturing photos or not
- `POST /start-stop-capture` to tell the device start or stop capturing photos

### Powering the Device

The ESP32-CAM can be powered using:

- Lithium Batteries: Provides a portable power source for field applications.
- Power Bank: Ideal for longer durations and higher power capacity.

### License

This project is licensed under the MIT License - see the LICENSE file for details.

### Acknowledgments

- PlatformIO
- ESPAsyncWebServer
- AI-Thinker for the ESP32-CAM board design

### Contact

For any questions or issues, please open an issue in the repository or contact the project maintainer.

## Disclaimer

This project is intended for hobby and educational purposes only. It is not designed, developed, or tested to meet the standards and requirements of a production-grade system. There are no guarantees, warranties, or assurances provided regarding the performance, reliability, or security of this code. The author assumes no responsibility or liability for any damages, losses, or other issues that may arise from using this source code. Use at your own risk.
