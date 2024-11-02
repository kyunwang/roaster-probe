import { serve } from '@hono/node-server';
import { createNodeWebSocket } from '@hono/node-ws';
import { Hono } from 'hono';

interface RoasterSensors {
	BT?: number;
	ET?: number;
}

interface ArtisanMessage {
	command: 'getData' | 'setData';
	id: number;
	roasterID?: number;
	data: RoasterSensors;
}

const app = new Hono();
const { injectWebSocket, upgradeWebSocket } = createNodeWebSocket({ app });

const roasterData: RoasterSensors = {};

app.get(
	'/artisan',
	upgradeWebSocket(() => ({
		onOpen() {
			console.log('Connection opened');
		},
		onClose() {
			console.log('Connection closed');
		},
		onMessage(event, ws) {
			try {
				if (typeof event.data !== 'string') return;

				const parsed: ArtisanMessage = JSON?.parse(event.data);

				switch (parsed.command) {
					case 'getData': {
						if (!roasterData.BT) return;

						const { BT, ET } = roasterData;

						const output = JSON.stringify({
							id: parsed.id,
							data: { BT, ET },
						});

						ws.send(output);
						break;
					}

					case 'setData': {
						let key: keyof RoasterSensors;

						for (key in parsed.data) {
							if (Object.prototype.hasOwnProperty.call(parsed.data, key)) {
								const sensorData = parsed.data[key];

								if (!sensorData) return;

								roasterData[key] = sensorData;
							}
						}

						break;
					}

					default:
						console.log('Message type: ', event.data);
						break;
				}
			} catch (err) {
				console.error(err);
			}
		},
	})),
);

const PORT = 8080;
console.log(`Server is running on http://localhost:${PORT}`);

const server = serve({
	fetch: app.fetch,
	port: PORT,
});

injectWebSocket(server);
