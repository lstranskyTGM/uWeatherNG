# local_broker

This Docker Compose configuration includes a local MQTT broker, creating a self-contained system for data exchange. This setup is best suited for standalone deployments or when data sovereignty and network independence are priorities.

## Cloudflared (Optional)

This optional tunneling service securely exposes your local server to the internet via the Cloudflare network, enhancing data privacy and integrity. It can be disabled for closed-network environments to streamline operations.

## Mosquitto (Optional)

An efficient, lightweight MQTT broker that enables real-time communication between IoT devices. It is particularly useful in localized networks or for testing purposes and can be disabled if not needed.

## Telegraf

A plugin-driven server agent that collects metrics from diverse sources including system metrics, logs, and sensors. Data is forwarded to InfluxDB for detailed analysis, optimized for scalability and flexibility in data handling.

## InfluxDB

A specialized time-series database that excels in storing and retrieving structured data points efficiently. It supports complex queries and real-time analytics, making it perfect for high-velocity data environments.

## Grafana

A powerful visualization tool that creates intuitive dashboards from data stored in InfluxDB. It helps monitor system performance and identify trends, aiding in decision-making and operational oversight.

## httpd

This web server component acts as the front end, offering real-time interaction and visualization of the data through a user-friendly web interface.