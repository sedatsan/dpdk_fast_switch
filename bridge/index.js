const net = require('net');
const http = require('http');
const { Server } = require('socket.io');

const httpServer = http.createServer();
const io = new Server(httpServer, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    }
});

const SOCKET_PATH = '/tmp/dpdk_switch.sock';

function connectToBackend() {
    console.log('Connecting to C backend...');
    const client = net.createConnection(SOCKET_PATH);

    client.on('connect', () => {
        console.log('Connected to C backend');
    });

    client.on('data', (data) => {
        try {
            const stats = JSON.parse(data.toString());
            console.log('Stats received:', stats);
            io.emit('stats', stats);
        } catch (e) {
            // Buffer might contain partial JSON
        }
    });

    client.on('error', (err) => {
        console.error('Socket error:', err.message);
    });

    client.on('close', () => {
        console.log('Backend connection closed. Retrying in 2s...');
        setTimeout(connectToBackend, 2000);
    });
}

io.on('connection', (socket) => {
    console.log('Web client connected');
});

httpServer.listen(3001, () => {
    console.log('Bridge listening on port 3001');
    connectToBackend();
});
