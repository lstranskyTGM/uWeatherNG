# DockerCompose

This system offers a ready-to-deploy solution that runs seamlessly across different environments supporting Docker and Docker Compose. It effectively orchestrates all containers, ensuring consistent performance and reliability. The setup boosts portability, scalability, and maintainability, ideal for demanding operational environments.

## Setup

1. Ensure Docker and Docker Compose are installed on your system.
2. Clone the repository to your local machine.
3. Navigate to the `DockerCompose` directory.
4. Configure the docker-compose.yml file to suit your environment.
5. Run the following command to start the system:

```bash
docker-compose up -d
```

6. Create Tokens for Grafana and Telegraf
7. Update the Telegraf configuration file with the InfluxDB token
8. Configure Grafana to connect to InfluxDB

## Configuration

### Docker Compose

Set credentials and environment variables in the `docker-compose.yml` file.

### Cloudflared (Optional)

Configure the tunnel `your-tunnel-id.json` and `config.yml` files.

### Mosquitto (Optional)

Configure the `mosquitto.conf` file with the desired settings.

### Telegraf

Configure the `telegraf.conf` file with the InfluxDB token.
If you are using the external MQTT broker change the broker address.
Change any other settings as needed. 

## Structure

The system includes the following components:

- **Cloudflared: (Optional)** A tunneling service that securely exposes the local server to the internet.
- **Mosquitto: (Optional)** An MQTT broker that enables real-time communication between IoT devices.
- **Telegraf:** A plugin-driven server agent for collecting and reporting metrics.
- **InfluxDB:** A time-series database that stores data points.
- **Grafana:** A visualization tool that creates dashboards for monitoring and analyzing data.
- **httpd:** A web server that provides access to a web page.

## Data Flow

The ESP32 device collects environmental data from various sensors and transmits it to the MQTT broker using the MQTT protocol. The data is then forwarded to the Telegraf agent, which processes and sends it to the InfluxDB database for storage. Grafana connects to the database to retrieve the data points and displays them on a dashboard for monitoring and analysis. Users can access the dashboard or a web page hosted by the httpd server to viev the real-time data.

## Line Protocol

The Line Protocol is a text-based format for writing data points to the InfluxDB database. It consists of a series of fields and tags that define the structure of the data points. The Line Protocol is used to insert, update, and delete data points in the database, enabling efficient storage and retrieval of time-series data.

### Format

````plaintext
<measurement>[,<tag-key>=<tag-value>,...] [<field-key>=<field-value>,...] [<timestamp>]
````

### Example 

```plaintext
# With Timestamp
weather,location=Vienna temperature=25,humidity=50,pressure=1013 1626825600000000000

# Without Timestamp
weather,location=Vienna temperature=25,humidity=50,pressure=1013
```

## Possible Extensions

- [ ] Add dashboard data for grafana