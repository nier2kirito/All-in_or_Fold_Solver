# Real-time MCCFR Training Visualization

This system provides **live, real-time visualization** of MCCFR training progress, showing Mean Absolute Error (MAE) and player utilities updating as training progresses.

## 🎯 Features

### Live Plots
- **Mean Absolute Error (MAE)**: Convergence progress in real-time
- **Zero-Sum Verification**: |Sum of utilities| (should stay near 0)
- **Individual Player Utilities**: All 4 players' utilities over time
- **Training Statistics**: Current iteration, MAE, utilities, and progress

### Real-time Updates
- **Automatic data refresh**: Monitors CSV file for new data
- **Smooth animations**: Updates every 50-100ms for fluid visualization
- **Live statistics**: Current values displayed alongside plots
- **Zero-sum indicator**: ✓/✗ indicator for mathematical correctness

## 🚀 Quick Start

### Method 1: Automatic Launcher (Recommended)
```bash
# Start both training and visualization automatically
python start_realtime_training.py -i 5000

# With custom parameters
python start_realtime_training.py -i 10000 --data-file my_training.csv
```

### Method 2: Manual Setup
```bash
# Terminal 1: Start the visualizer
python realtime_visualizer.py

# Terminal 2: Start training with real-time mode
./build/bin/aof_mccfr --realtime -i 5000 --log-interval 5
```

### Method 3: Command Line Integration
```bash
# Enable real-time mode directly
./build/bin/aof_mccfr --realtime -i 5000 -q
```

## 📊 What You'll See

### 1. Mean Absolute Error Plot (Top Left)
- **Y-axis**: MAE (log scale)
- **Trend**: Should decrease over time as strategies improve
- **Goal**: Lower values indicate better convergence

### 2. Zero-Sum Verification (Top Right)  
- **Y-axis**: |Sum of utilities| (log scale)
- **Expected**: Should stay near 1e-15 (floating-point precision)
- **Indicator**: ✓ if sum < 1e-10, ✗ otherwise

### 3. Player Utilities (Bottom Left)
- **Lines**: SB (red), BB (blue), CO (green), BTN (orange)
- **Behavior**: Should stabilize as Nash equilibrium is reached
- **Zero line**: Black dashed line for reference

### 4. Training Statistics (Bottom Right)
- **Current iteration** and progress
- **Live MAE** and utility sum
- **Individual player utilities**
- **Data points** collected and zero-sum status

## ⚙️ Configuration Options

### Training Configuration
```bash
./build/bin/aof_mccfr \
    --realtime \              # Enable real-time mode
    -i 10000 \               # Number of iterations
    --log-interval 5 \       # Log every 5 iterations (faster updates)
    -q                       # Quiet mode (less console output)
```

### Visualizer Configuration
```bash
python realtime_visualizer.py \
    --csv training_data.csv \     # CSV file to monitor
    --max-points 1000 \           # Maximum data points to display
    --update-interval 50          # Update frequency (ms)
```

### Launcher Configuration
```bash
python start_realtime_training.py \
    -i 5000 \                     # Training iterations
    --data-file my_data.csv       # Custom data file name
```

## 🔧 Advanced Usage

### High-Frequency Updates
```bash
# Very frequent logging for smooth visualization
./build/bin/aof_mccfr --realtime -i 10000 --log-interval 1
```

### Long Training Sessions
```bash
# Efficient settings for long training
python start_realtime_training.py -i 100000
python realtime_visualizer.py --max-points 5000 --update-interval 100
```

### Multiple Experiments
```bash
# Run multiple experiments with different parameters
./build/bin/aof_mccfr --realtime -i 5000 --log-interval 3 -o experiment1 &
sleep 2
python realtime_visualizer.py --csv training_data.csv
```

## 🎮 Interactive Features

### During Visualization
- **Zoom**: Mouse wheel to zoom in/out on plots
- **Pan**: Click and drag to pan around
- **Legend**: Click legend items to show/hide lines
- **Window**: Resize window for better viewing

### Keyboard Shortcuts
- **Ctrl+C**: Stop both training and visualization
- **Close window**: Stop visualization only (training continues)

## 📈 Interpreting the Results

### Convergence Indicators
- **Decreasing MAE**: Algorithm is learning and improving
- **Stable utilities**: Players' strategies are converging
- **Consistent zero-sum**: Game logic is mathematically correct

### Typical Patterns
- **Early training**: High MAE, volatile utilities
- **Mid training**: Decreasing MAE, stabilizing utilities  
- **Late training**: Low MAE, stable utilities near Nash equilibrium

### Warning Signs
- **Increasing MAE**: Possible learning rate issues
- **Large utility sums**: Game logic bug (should be ~1e-15)
- **Oscillating utilities**: May need more iterations

## 🛠️ Troubleshooting

### Visualizer Issues
```bash
# Check if CSV file exists
ls -la training_data.csv

# Verify Python dependencies
pip install matplotlib pandas numpy

# Test with existing data
python realtime_visualizer.py build/training_data.csv
```

### Training Issues
```bash
# Check executable
ls -la build/bin/aof_mccfr

# Test basic training
./build/bin/aof_mccfr -i 100 -q

# Verify file permissions
chmod +x build/bin/aof_mccfr
```

### Synchronization Issues
```bash
# Manual synchronization test
./build/bin/aof_mccfr --realtime -i 1000 --log-interval 10 &
sleep 2
python realtime_visualizer.py --update-interval 200
```

## 📝 Example Output

### Console Output (Training)
```
Real-time visualization mode enabled!
Start the visualizer: python realtime_visualizer.py

Starting MCCFR training with 5000 iterations...
Data logging enabled: training_data.csv
Real-time visualization mode: Start 'python realtime_visualizer.py' in another terminal
```

### Console Output (Visualizer)
```
Starting real-time visualization...
Monitoring: training_data.csv
Update interval: 50ms
Waiting for training data...
✓ New data detected: 50 points
✓ New data detected: 100 points
...
```

## 🎯 Performance Tips

### For Smooth Visualization
- Use `--log-interval 3-10` for good balance
- Set `--update-interval 50-100` in visualizer
- Limit `--max-points 1000-2000` for responsiveness

### For Long Training
- Use `--log-interval 25-100` to reduce file I/O
- Set `--update-interval 200-500` for efficiency
- Increase `--max-points 5000+` to see full history

### For Analysis
- Save data with `--data-file descriptive_name.csv`
- Use post-training analysis: `python plot_training_data.py data.csv`
- Keep visualizer running after training to examine final state

## 📊 Data Format

The CSV file contains:
```csv
iteration,mean_absolute_error,utility_sum,player_0_utility,player_1_utility,player_2_utility,player_3_utility,elapsed_time_ms
10,0.6300000000,-2.22e-16,-0.1200000000,-1.1400000000,1.2600000000,0.0000000000,8
20,0.3850000000,-1.11e-16,-0.1200000000,-0.6500000000,0.7700000000,0.0000000000,15
...
```

## 🎉 Success Indicators

You'll know it's working when you see:
- **Smooth, updating plots** in the visualizer window
- **Decreasing MAE trend** over time
- **Zero-sum verification** showing ✓ consistently
- **Stable final utilities** that sum to ~0

The real-time visualization makes MCCFR training engaging and provides immediate feedback on convergence quality!
