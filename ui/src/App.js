import React, { useEffect, useState } from 'react';
import io from 'socket.io-client';

const SOCKET_URL = process.env.REACT_APP_SOCKET_URL || 'http://localhost:3001';
const socket = io(SOCKET_URL);

const StatCard = ({ label, value, color = 'white' }) => (
    <div style={{ padding: '1rem', backgroundColor: '#2d3748', borderRadius: '0.5rem', boxShadow: '0 4px 6px rgba(0,0,0,0.1)' }}>
        <h2 style={{ fontSize: '1.25rem', color: '#a0aec0' }}>{label}</h2>
        <p style={{ fontSize: '1.5rem', fontWeight: 'bold', color }}>{value}</p>
    </div>
);

function App() {
    const [stats, setStats] = useState({ ports: [], mac_table_entries: 0 });
    const [isConnected, setIsConnected] = useState(socket.connected);

    useEffect(() => {
        const onConnect = () => setIsConnected(true);
        const onDisconnect = () => setIsConnected(false);
        const onStats = (data) => setStats(data);

        socket.on('connect', onConnect);
        socket.on('disconnect', onDisconnect);
        socket.on('stats', onStats);

        return () => {
            socket.off('connect', onConnect);
            socket.off('disconnect', onDisconnect);
            socket.off('stats', onStats);
        };
    }, []);

    return (
        <div style={{ padding: '2rem', backgroundColor: '#1a202c', color: 'white', minHeight: '100vh', fontFamily: 'sans-serif' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '2rem' }}>
                <h1 style={{ fontSize: '2rem', fontWeight: 'bold' }}>DPDK L2 Switch Monitor</h1>
                <div style={{ display: 'flex', alignItems: 'center' }}>
                    <div style={{ 
                        width: '12px', height: '12px', borderRadius: '50%', 
                        backgroundColor: isConnected ? '#48bb78' : '#f56565',
                        marginRight: '8px'
                    }} />
                    <span>{isConnected ? 'Connected' : 'Disconnected'}</span>
                </div>
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))', gap: '1.5rem' }}>
                <StatCard 
                    label="MAC Table Size" 
                    value={stats.mac_table_entries} 
                    color="#ecc94b"
                />
                
                {stats.ports.map(p => (
                    <StatCard 
                        key={p.id}
                        label={`Port ${p.id} Throughput`}
                        value={`${p.pps.toLocaleString()} pkts/s`}
                        color="#63b3ed"
                    />
                ))}
            </div>
        </div>
    );
}

export default App;
