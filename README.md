# HTTP Server (Linux/Unix Systems)

## Overview
This project features a custom-built HTTP server in C, designed to serve my portfolio website hosted at [calum-crawford.com](https://calum-crawford.com). The server is implemented using the `poll()` function and adheres to proper HTTP protocols. It's capable of running on localhost, allowing for easy testing and deployment of web content.

## Features
- **Custom HTTP Server**: Built in C with `poll()`.
- **Local Hosting**: Serve your portfolio on localhost.
- **Flexibility**: Changeable port option as per user requirement.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Before running the server, ensure you have `git` and `make` installed on your machine.

### Installation

1. **Clone the Repository**

   Open your terminal and clone the repository using the following command:
    ```bash
    git clone https://github.com/ccrawford4/HTTP-Server.git
    ```

2. **Build the Server**

   Navigate to the cloned directory and compile the server:
    ```bash
    cd HTTP-Server
    make
    ```

### Running the Server

- **Start the Server**

  To start the server and see print statements, use:
    ```bash
    ./server
    ```

  To run the server in the background, use:
    ```bash
    ./server &
    ```

- **Accessing the Website**

  Open a web browser and visit:
    ```
    localhost:8008
    ```
  Or replace `8008` with the port number specified in `port.txt` if you have changed it.

The resulting page should show up in your browser:
![./image.png(./image.png)]

### Customizing the Page

To customize the content served by your HTTP server,
you can download any website of your choice using `wget` with
the `--recursive`, `--page-prerequisites`, and `--convert-links` options.
This allows you to download the complete website, with its links modified for local viewing.
You can then move the downloaded content to the `www` directory of your server using
Unix/Linux `mv` commands, enabling you to serve that website.

#### Steps to Customize:

1. **Download a Website**

   Choose the website you wish to serve. You can download it for offline access using `wget` with the following options:
   ```bash
   wget --recursive --page-requisites --convert-links [website-url
   ```
   ##### Disclaimer: 
    Ensure you have the appropriate permissions to download and use the content of the website. This code and its developers are not responsible for any unlawful download or distribution of copyrighted material. It is the user's responsibility to adhere to all applicable laws and regulations regarding website content usage.

### Customizing the Port

To change the server's port:
1. Edit the `port.txt` file and enter your desired port number.
2. Restart the server for the changes to take effect.
---
