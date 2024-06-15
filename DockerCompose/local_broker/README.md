# local_broker

This Docker Compose setup includes a local MQTT broker for data exchange. The system is self-contained and does not require an external MQTT broker, making it suitable for standalone deployments.

### Cloudflared (Optional)

Cloudflared is a tunneling service that securely exposes the local server to the internet. It provides a secure connection between the server and the Cloudflare network, ensuring data privacy and integrity. The service is optional and can be disabled if not required.

### Mosquitto (Optional)

Mosquitto is a lightweight MQTT broker that provides a messaging protocol for IoT devices. It enables devices to communicate with each other over a network, facilitating real-time data exchange. The service is optional and can be disabled if not required.

### Telegraf

Telegraf is a plugin-driven server agent for collecting and reporting metrics. It collects data from various sources, such as system metrics, logs, and sensors, and sends it to the InfluxDB database for storage and analysis.

### InfluxDB

InfluxDB is a time-series database that stores data points in a structured format. It provides efficient storage and retrieval of time-series data, making it ideal for monitoring and analytics applications.

### Grafana

Grafana is a visualization tool that creates dashboards for monitoring and analyzing data. It connects to the InfluxDB database to retrieve data points and displays them in a user-friendly format, enabling users to track performance and identify trends.

### httpd

httpd is a web server that provides access to a web page. It serves as the front end for the system, allowing users to interact with the data and visualize it in real time.