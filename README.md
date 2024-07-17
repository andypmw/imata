# iMata

iMata is a hobby project focused on programming an embedded device, ESP32-Camera, using C++ and creating a mobile application using React Native. This project is divided into two main parts:

- `imata-device-esp32cam`: Code for the ESP32-Camera device
- `imata-app`: Code for the React Native mobile application

**Warning:** This is not a production-grade source code. Use it for educational and hobby purposes only.

## Project Structure

```
imata/
├── imata-device-esp32cam
│ └── // C++ code for the ESP32-Camera device (using Platform-IO)
└── imata-app
└── // React Native code for the mobile application (using Expo)
```

## imata-device-esp32cam

This folder contains the C++ code for the ESP32-Camera device. The ESP32-Camera is programmed to capture images and perform specific tasks.

### Features

- Capture images every second using the ESP32-Camera module
- Save images in JPEG format to a MicroSD card
- Set up a WiFi access point
- Provide a REST API to control the device and browse captured photos

## System Architecture

To know how the iMata ESP32-CAM device and iMata app work together, you can check the following architecture diagram.

![iMata system architecture diagram](_assets/imata-architecture.png?raw=true "iMata system architecture diagram")

### Setup

1. Install the ESP32 add-on for the Arduino IDE.
2. Clone this repository.
3. Open the `imata-device-esp32cam` folder in the Arduino IDE.
4. Select the correct board and port in the Arduino IDE.
5. Upload the code to your ESP32-Camera module.

### Usage

After uploading the code to the ESP32-Camera, the device will:

1. Capture images every second.
2. Save images with a watermark saying "captured by iMata - Andy Primawan" to a MicroSD card.
3. Create a WiFi access point.
4. Provide a REST API to control the device and browse captured photos.

## imata-app

This folder contains the React Native code for the mobile application. The app is designed to interact with the ESP32-Camera device, control its functions, and display the captured images.

### Features

- Connect to the ESP32-Camera's WiFi access point
- Control the ESP32-Camera device
- Browse and display JPEG images captured by the ESP32 device

### Setup

1. Install Node.js and npm.
2. Install React Native CLI.
3. Clone this repository.
4. Navigate to the `imata-app` folder.
5. Run `npm install` to install dependencies.
6. Run `npm start` to start the development server.

### Usage

1. Ensure the ESP32-Camera is running and the WiFi access point is active.
2. Connect your mobile device to the ESP32-Camera's WiFi network.
3. Open the app to control the ESP32-Camera and view the images it captures.

## Contributing

Feel free to fork this repository and contribute by submitting pull requests. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

This project is intended for hobby and educational purposes only. It is not designed, developed, or tested to meet the standards and requirements of a production-grade system. There are no guarantees, warranties, or assurances provided regarding the performance, reliability, or security of this code. The author assumes no responsibility or liability for any damages, losses, or other issues that may arise from using this source code. Use at your own risk.
