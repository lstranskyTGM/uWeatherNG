# DockerCompose

This system offers a ready-to-deploy solution that runs seamlessly across different environments supporting Docker and Docker Compose. It effectively orchestrates all containers, ensuring consistent performance and reliability. The setup boosts portability, scalability, and maintainability, ideal for demanding operational environments.

## Structure

The system includes the following components:

- **Cloudflared:** A tunneling service that securely exposes the local server to the internet.
- **Telegraf:** A plugin-driven server agent for collecting and reporting metrics.
- **InfluxDB:** A time-series database that stores data points.
- **Grafana:** A visualization tool that creates dashboards for monitoring and analyzing data.
- **httpd:** A web server that provides access to a web page.

## Setup

1. Ensure Docker and Docker Compose are installed on your system.
2. Clone the repository to your local machine.
3. Navigate to the `DockerCompose` directory.
4. Configure the docker-compose.yml file to suit your environment.
5. Run the following command to start the system:

```bash
docker-compose up -d
```

## Configuration

## Containers

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

## Data Flow

The ESP32 device collects environmental data from various sensors and transmits it to the MQTT broker using the MQTT protocol. The data is then forwarded to the Telegraf agent, which processes and sends it to the InfluxDB database for storage. Grafana connects to the database to retrieve the data points and displays them on a dashboard for monitoring and analysis. Users can access the dashboard or a web page hosted by the httpd server to viev the real-time data.

## Line Protocol

The Line Protocol is a text-based format for writing data points to the InfluxDB database. It consists of a series of fields and tags that define the structure of the data points. The Line Protocol is used to insert, update, and delete data points in the database, enabling efficient storage and retrieval of time-series data.

### Format

````plaintext
<measurement>[,<tag-key>=<tag-value>[,<tag-key>=<tag-value>...]] <field-key>=<field-value>[,<field-key>=<field-value>...] [<timestamp>]
````

### Example

```plaintext
# With Timestamp
weather,location=Vienna temperature=25,humidity=50,pressure=1013 1626825600000000000

# Without Timestamp
weather,location=Vienna temperature=25,humidity=50,pressure=1013
```

## Possible Extensions