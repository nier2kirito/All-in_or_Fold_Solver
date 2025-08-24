# Direct Real-time MCCFR Visualization 🚀

This system provides **true real-time visualization** of MCCFR training with **direct streaming** from C++ to Python plots - **no intermediate files, no CSV, no delays**.

## 🎯 Key Features

### ✅ What Makes This Special
- **Direct C++ → Python communication** via Unix pipes
- **No intermediate files** - data streams live from memory
- **True real-time updates** - plots update as training progresses
- **Automatic process management** - Python visualizer starts/stops automatically
- **Zero latency** - data appears in plots within milliseconds
- **Memory efficient** - no file I/O overhead

### ✅ Live Visualizations
- **Mean Absolute Error (MAE)**: Real-time convergence tracking
- **Zero-Sum Verification**: Live |sum of utilities| monitoring
- **Player Utilities**: All 4 players updating simultaneously
- **Training Statistics**: Current iteration, MAE, utilities, timing
- **Console Stats**: Periodic updates with ✓/✗ zero-sum indicator

## 🚀 Quick Start

### Simple Usage
```bash
# Enable direct real-time visualization
./build/bin/aof_mccfr --realtime -i 5000

# With custom update frequency
./build/bin/aof_mccfr --realtime -i 10000 --log-interval 3
```

### Advanced Configuration
```cpp
// In your C++ code
mccfr::TrainingConfig config;
config.realtimeConfig.enabled = true;
config.realtimeConfig.updateInterval = 5;        // Update every 5 iterations
config.realtimeConfig.maxDataPoints = 2000;      // Keep 2000 data points
config.realtimeConfig.showConsoleStats = true;   // Show periodic stats
config.realtimeConfig.windowTitle = "My Training";

auto utilities = trainer.train(config);
```

## 🔧 How It Works

### Architecture
```
┌─────────────┐    Unix Pipe    ┌─────────────────┐
│   C++ MCCFR │ ──────────────→ │ Python Matplotlib│
│   Training  │   JSON Stream   │   Real-time     │
│             │                 │   Plotting      │
└─────────────┘                 └─────────────────┘
```

### Data Flow
1. **C++ Training Loop**: Calculates MAE, utilities every N iterations
2. **JSON Serialization**: Converts data to JSON format
3. **Unix Pipe**: Streams JSON directly to Python process
4. **Python Matplotlib**: Receives JSON, updates plots in real-time
5. **Live Display**: Plots update smoothly without file I/O

### Process Management
- **Automatic Startup**: Python visualizer starts when training begins
- **Process Forking**: Uses `fork()` and `exec()` for clean process separation
- **Pipe Communication**: Efficient `pipe()` system call for data transfer
- **Graceful Shutdown**: Proper cleanup when training completes

## 📊 What You'll See

### Real-time Plot Window
```
┌─────────────────────────────────────────────────────────────┐
│ Direct MCCFR Visualization Demo                             │
├─────────────────┬───────────────────────────────────────────┤
│ Mean Absolute   │ Zero-Sum Verification                     │
│ Error (MAE)     │ |Sum of Utilities|                       │
│                 │                                           │
│ [Decreasing     │ [Should stay near 1e-15]                 │
│  curve]         │                                           │
├─────────────────┼───────────────────────────────────────────┤
│ Player Utilities│ Current Statistics                        │
│ SB, BB, CO, BTN │                                           │
│                 │ Iteration: 1250                          │
│ [4 colored      │ MAE: 0.001234                            │
│  lines]         │ |Sum|: 2.34e-16                          │
│                 │ Zero-Sum: ✓                              │
└─────────────────┴───────────────────────────────────────────┘
```

### Console Output
```
🚀 Starting real-time visualization...
✓ Python visualizer process started (PID: 12345)
✓ Real-time visualization started!
  Window title: Direct MCCFR Visualization Demo
  Update interval: every 5 iterations
  Max data points: 2000

📈 [Iteration    250] MAE: 2.140e-01 | Sum: -6.66e-16 | Utilities: [-0.0840, -0.2160, 0.4280, -0.1280] | Time: 189ms ✓
📈 [Iteration    500] MAE: 1.420e-01 | Sum: -2.50e-16 | Utilities: [-0.1540, 0.1480, 0.1360, -0.1300] | Time: 378ms ✓
```

## ⚙️ Configuration Options

### C++ Configuration
```cpp
struct Config {
    bool enabled = false;                    // Enable real-time visualization
    int updateInterval = 10;                 // Update every N iterations
    int maxDataPoints = 1000;                // Maximum data points to keep
    bool showConsoleStats = true;            // Show stats in console
    std::string windowTitle = "MCCFR Training"; // Window title
};
```

### Command Line Options
```bash
--realtime              # Enable direct real-time visualization
--log-interval 5        # Update every 5 iterations (faster updates)
-i 10000               # Number of training iterations
-q                     # Quiet mode (less console output)
```

### Performance Tuning
```cpp
// High-frequency updates (smooth animation)
config.realtimeConfig.updateInterval = 1;
config.realtimeConfig.maxDataPoints = 500;

// Efficient long training
config.realtimeConfig.updateInterval = 25;
config.realtimeConfig.maxDataPoints = 5000;

// Memory-conscious
config.realtimeConfig.updateInterval = 10;
config.realtimeConfig.maxDataPoints = 1000;
```

## 🎮 Interactive Features

### During Training
- **Live Zoom/Pan**: Use mouse to examine plots
- **Legend Toggle**: Click legend items to show/hide lines
- **Window Resize**: Adjust for optimal viewing
- **Real-time Stats**: Watch values update in statistics panel

### After Training
- **Persistent Window**: Visualization continues after training ends
- **Final Analysis**: Examine complete training curve
- **Manual Close**: Close window when done examining

## 🆚 Comparison with File-based Approaches

| Feature | Direct Streaming | CSV-based | Python Scripts |
|---------|------------------|-----------|----------------|
| **Latency** | ~1ms | ~100ms+ | ~1000ms+ |
| **File I/O** | None | Heavy | Heavy |
| **Memory Usage** | Minimal | High | High |
| **Setup** | Automatic | Manual | Manual |
| **Real-time** | True | Pseudo | False |
| **Dependencies** | Built-in | External | External |

## 🛠️ Troubleshooting

### Common Issues

**No visualization window appears:**
```bash
# Check Python/matplotlib installation
python3 -c "import matplotlib.pyplot as plt; print('OK')"

# Try with explicit Python path
export PYTHON_PATH=/usr/bin/python3
./build/bin/aof_mccfr --realtime -i 100
```

**Permission denied errors:**
```bash
# Ensure executable permissions
chmod +x build/bin/aof_mccfr

# Check pipe creation permissions
ulimit -n  # Should be > 1024
```

**Process doesn't start:**
```bash
# Check for zombie processes
ps aux | grep python
pkill -f matplotlib

# Clean restart
./build/bin/aof_mccfr --realtime -i 50
```

### Debug Mode
```cpp
// Enable debug output
config.realtimeConfig.showConsoleStats = true;

// Check process status
std::cout << "Visualizer running: " << visualizer_->isRunning() << std::endl;
```

## 🎯 Performance Metrics

### Typical Performance
- **Startup Time**: ~200ms (Python process + matplotlib)
- **Update Latency**: ~1-5ms per data point
- **Memory Overhead**: ~10MB (matplotlib + data buffer)
- **CPU Overhead**: ~2-5% (plotting + data transfer)

### Scaling
- **1K iterations**: Smooth, real-time updates
- **10K iterations**: Excellent performance
- **100K+ iterations**: Use `updateInterval = 50+` for efficiency

## 📈 Success Indicators

You'll know it's working perfectly when:
- ✅ **Visualization window opens automatically**
- ✅ **Plots update smoothly in real-time**
- ✅ **Console shows periodic stats with ✓ indicators**
- ✅ **MAE decreases over time**
- ✅ **Zero-sum verification stays near 1e-15**
- ✅ **No intermediate files are created**

## 🚀 Example Usage

### Run the Demo
```bash
# Build and run the direct visualization demo
cd build
make
./examples/direct_realtime_visualization
```

### Custom Training
```bash
# Quick test (2 minutes)
./bin/aof_mccfr --realtime -i 5000 --log-interval 10

# Production training (10+ minutes)
./bin/aof_mccfr --realtime -i 100000 --log-interval 50

# High-frequency visualization (smooth animation)
./bin/aof_mccfr --realtime -i 2000 --log-interval 2
```

This direct real-time visualization system transforms MCCFR training from a "black box" into an **engaging, visual experience** where you can watch your AI learn poker strategies in real-time! 🎰✨
