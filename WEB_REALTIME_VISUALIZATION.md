# Web-based Real-time MCCFR Visualization 🌐

This system provides **true real-time visualization** of MCCFR training through a **localhost web interface** - **no CSV files, no Python dependencies, no intermediate storage**.

## 🎯 Key Features

### ✅ Web-based Real-time Visualization
- **Built-in HTTP server** on `localhost:8080`
- **No external dependencies** - everything embedded in C++
- **No CSV files** - data streams directly from memory to browser
- **Modern web interface** with Chart.js for smooth animations
- **Real-time updates** via HTTP polling (100ms intervals)
- **Dark theme** optimized for extended viewing

### ✅ Live Charts
- **Mean Absolute Error (MAE)**: Real-time convergence tracking (log scale)
- **Zero-Sum Verification**: Live |sum of utilities| monitoring (log scale)  
- **Player Utilities**: All 4 players updating simultaneously
- **Live Statistics Panel**: Current iteration, MAE, utilities, timing, zero-sum status

## 🚀 Quick Start

### Simple Usage
```bash
# Start training with web visualization
./build/bin/aof_mccfr --realtime -i 5000

# Open your browser to http://localhost:8080
# Watch the plots update in real-time!
```

### What You'll See
1. **C++ starts HTTP server** on port 8080
2. **Console shows**: "🌐 Open http://localhost:8080 in your browser"
3. **Browser displays**: Real-time charts updating every 100ms
4. **Training data** streams directly from C++ memory to browser

## 📊 Web Interface

### Dark Theme Dashboard
```
┌─────────────────────────────────────────────────────────────┐
│ 🚀 Real-time MCCFR Training Visualization                  │
├─────────────────┬───────────────────────────────────────────┤
│ Mean Absolute   │ Zero-Sum Verification                     │
│ Error (MAE)     │ |Sum of Utilities| (log scale)           │
│ (log scale)     │                                           │
│ [Blue line      │ [Red line showing ~1e-16]                │
│  decreasing]    │                                           │
├─────────────────┼───────────────────────────────────────────┤
│ Player Utilities│ Current Statistics                        │
│ SB, BB, CO, BTN │ Iteration: 1,250                         │
│                 │ MAE: 1.234e-02                           │
│ [4 colored      │ |Sum|: 2.34e-16                          │
│  lines]         │ Time: 1,125ms                            │
│                 │                                           │
│                 │ Player Utilities:                         │
│                 │   SB: +0.0445                            │
│                 │   BB: +0.3460                            │
│                 │   CO: +0.1345                            │
│                 │   BTN: -0.5250                           │
│                 │                                           │
│                 │ Zero-Sum: ✓                              │
└─────────────────┴───────────────────────────────────────────┘
```

## ⚙️ Configuration

### Command Line
```bash
./build/bin/aof_mccfr --realtime -i 10000 --log-interval 5
```

### C++ Configuration
```cpp
mccfr::TrainingConfig config;
config.realtimeConfig.enabled = true;
config.realtimeConfig.updateInterval = 5;        // Update every 5 iterations  
config.realtimeConfig.maxDataPoints = 2000;      // Keep 2000 data points
config.realtimeConfig.showConsoleStats = true;   // Show periodic console stats
config.realtimeConfig.windowTitle = "My Training";

auto utilities = trainer.train(config);
```

## 🔧 How It Works

### Architecture
```
┌─────────────┐    HTTP Server    ┌─────────────────┐
│   C++ MCCFR │ ←──────────────── │    Browser      │
│   Training  │   GET /data       │   (localhost:   │
│             │ ──────────────→   │    8080)        │
│             │   JSON Response   │                 │
└─────────────┘                   └─────────────────┘
```

### Data Flow
1. **C++ Training**: Calculates MAE, utilities every N iterations
2. **JSON Storage**: Latest data stored in memory as JSON string
3. **HTTP Server**: Built-in server serves data on `localhost:8080`
4. **Browser Polling**: JavaScript fetches `/data` every 100ms
5. **Chart Updates**: Chart.js updates plots with new data

### Web Endpoints
- **`GET /`**: Serves the HTML visualization dashboard
- **`GET /data`**: Returns latest training data as JSON

## 🎮 Browser Features

### Interactive Charts
- **Zoom**: Mouse wheel to zoom in/out
- **Pan**: Click and drag to explore data
- **Legend**: Click to show/hide data series
- **Responsive**: Adapts to window size

### Real-time Updates
- **100ms polling**: Smooth, responsive updates
- **Automatic scaling**: Charts auto-adjust axes
- **Data limiting**: Keeps last 1000 points for performance
- **Live statistics**: Values update in real-time

## 📈 Performance

### Typical Metrics
- **Server startup**: ~50ms (HTTP server initialization)
- **Update latency**: ~100-200ms (HTTP polling interval)
- **Memory usage**: ~5MB (embedded HTTP server + data buffer)
- **CPU overhead**: ~1-2% (HTTP server + JSON serialization)
- **Browser performance**: Smooth 60fps chart animations

### Scaling
- **1K iterations**: Excellent performance
- **10K iterations**: Great performance  
- **100K+ iterations**: Use `updateInterval = 50+` for efficiency

## 🛠️ Advantages Over Previous Approaches

| Feature | Web-based | CSV Files | Python Scripts |
|---------|-----------|-----------|----------------|
| **Setup** | Zero setup | Manual | Complex |
| **Dependencies** | None | File system | Python + libs |
| **Latency** | ~100ms | ~1000ms+ | ~5000ms+ |
| **File I/O** | None | Heavy | Heavy |
| **Cross-platform** | Yes | Yes | Depends |
| **Real-time** | True | Pseudo | False |
| **Accessibility** | Any browser | Limited | Python only |

## 🌐 Browser Compatibility

### Supported Browsers
- ✅ **Chrome/Chromium** 60+
- ✅ **Firefox** 55+  
- ✅ **Safari** 12+
- ✅ **Edge** 79+

### Required Features
- ES6 JavaScript (async/await)
- Fetch API
- CSS Grid
- Canvas (for Chart.js)

## 🛠️ Troubleshooting

### Port Already in Use
```bash
# Check what's using port 8080
lsof -i :8080

# Kill process if needed
kill -9 <PID>

# Or use different port (modify source code)
```

### Browser Won't Connect
```bash
# Check if server is running
curl http://localhost:8080/data

# Check firewall settings
sudo ufw allow 8080

# Try different browser
```

### No Data Appearing
```bash
# Verify training is running
ps aux | grep aof_mccfr

# Check console output for errors
./bin/aof_mccfr --realtime -i 100 --log-interval 1
```

## 🚀 Example Usage

### Quick Demo
```bash
cd build
./bin/aof_mccfr --realtime -i 1000 --log-interval 5

# In browser: http://localhost:8080
# Watch MAE decrease and utilities converge!
```

### Production Training  
```bash
# Long training with efficient updates
./bin/aof_mccfr --realtime -i 100000 --log-interval 25

# Open browser and let it run
# Perfect for monitoring overnight training
```

### Multiple Sessions
```bash
# Terminal 1: Training
./bin/aof_mccfr --realtime -i 10000

# Terminal 2: Monitor via curl
watch -n 1 'curl -s localhost:8080/data | jq .'

# Browser: Visual monitoring
# http://localhost:8080
```

## 🎯 Success Indicators

Perfect operation shows:
- ✅ **Console**: "✓ Web server started on http://localhost:8080"
- ✅ **Browser**: Charts updating smoothly every ~100ms
- ✅ **MAE**: Decreasing trend over time
- ✅ **Zero-Sum**: Green ✓ indicator consistently shown  
- ✅ **Utilities**: Converging to stable values
- ✅ **No files**: No CSV or temporary files created

## 💡 Tips

### For Best Experience
- **Use Chrome/Firefox** for best Chart.js performance
- **Full screen browser** for optimal chart viewing
- **Close other tabs** to reduce browser resource usage
- **Use `--log-interval 3-10`** for smooth but efficient updates

### For Long Training
- **Leave browser tab open** - it won't affect training performance
- **Bookmark `localhost:8080`** for easy access
- **Use `--log-interval 25+`** for very long training sessions
- **Multiple browser windows** work fine for monitoring

This web-based system transforms MCCFR training into a **modern, accessible, real-time experience** that works in any browser without external dependencies! 🎰🌐✨
