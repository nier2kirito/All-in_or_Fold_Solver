/**
 * @file realtime_visualizer.cpp
 * @brief Implementation of direct real-time visualization for MCCFR training
 */

#include "mccfr/realtime_visualizer.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <cmath>

namespace mccfr {

RealtimeVisualizer::RealtimeVisualizer() : config_() {
    // Default constructor
}

RealtimeVisualizer::RealtimeVisualizer(const Config& config) : config_(config) {
    // Constructor with config
}

RealtimeVisualizer::~RealtimeVisualizer() {
    stop();
}

bool RealtimeVisualizer::start() {
    if (running_ || !config_.enabled) {
        return false;
    }
    
    std::cout << "🚀 Starting real-time visualization...\n";
    
    // Start web server for localhost visualization
    startWebServer();
    
    // Start visualization thread
    running_ = true;
    shouldStop_ = false;
    visualizationThread_ = std::thread(&RealtimeVisualizer::visualizationLoop, this);
    
    std::cout << "✓ Real-time visualization started!\n";
    std::cout << "  Window title: " << config_.windowTitle << "\n";
    std::cout << "  Update interval: every " << config_.updateInterval << " iterations\n";
    std::cout << "  Max data points: " << config_.maxDataPoints << "\n\n";
    
    return true;
}

void RealtimeVisualizer::stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "\n🛑 Stopping real-time visualization...\n";
    
    // Signal threads to stop
    shouldStop_ = true;
    dataCondition_.notify_all();
    
    // Wait for visualization thread
    if (visualizationThread_.joinable()) {
        visualizationThread_.join();
    }
    
    // Stop web server
    stopWebServer();
    
    running_ = false;
    std::cout << "✓ Real-time visualization stopped.\n";
}

void RealtimeVisualizer::addDataPoint(const TrainingDataPoint& dataPoint) {
    if (!running_) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        
        // Add new data point
        dataPoints_.push_back(dataPoint);
        
        // Limit data points to prevent memory growth
        while (dataPoints_.size() > static_cast<size_t>(config_.maxDataPoints)) {
            dataPoints_.pop_front();
        }
    }
    
    // Notify visualization thread
    dataCondition_.notify_one();
    
    // Send data to web clients
    if (webServerRunning_) {
        broadcastDataToClients(dataPoint);
    }
    
    // Print console stats if enabled
    if (config_.showConsoleStats && dataPoint.iteration % (config_.updateInterval * 10) == 0) {
        printConsoleStats(dataPoint);
    }
}

void RealtimeVisualizer::visualizationLoop() {
    std::cout << "📊 Visualization loop started\n";
    
    while (!shouldStop_) {
        std::unique_lock<std::mutex> lock(dataMutex_);
        
        // Wait for new data or stop signal
        dataCondition_.wait(lock, [this] { 
            return !dataPoints_.empty() || shouldStop_; 
        });
        
        if (shouldStop_) {
            break;
        }
        
        // Process available data points
        if (!dataPoints_.empty()) {
            updatePlots();
        }
        
        lock.unlock();
        
        // Small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "📊 Visualization loop stopped\n";
}

void RealtimeVisualizer::updatePlots() {
    // This method would update the plots
    // For now, we rely on the Python subprocess
    // In a more advanced implementation, we could use a C++ plotting library
}

void RealtimeVisualizer::printConsoleStats(const TrainingDataPoint& latest) {
    std::cout << "📈 [Iteration " << std::setw(6) << latest.iteration << "] ";
    std::cout << "MAE: " << std::scientific << std::setprecision(3) << latest.meanAbsoluteError << " | ";
    std::cout << "Sum: " << std::setprecision(2) << latest.utilitySum << " | ";
    std::cout << "Utilities: [";
    
    for (size_t i = 0; i < latest.playerUtilities.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed << std::setprecision(4) << latest.playerUtilities[i];
    }
    
    std::cout << "] | Time: " << latest.elapsedTime.count() << "ms";
    
    // Zero-sum indicator
    if (std::abs(latest.utilitySum) < 1e-10) {
        std::cout << " ✓";
    } else {
        std::cout << " ✗";
    }
    
    std::cout << "\n";
}

void RealtimeVisualizer::startWebServer() {
    if (webServerRunning_) {
        return;
    }
    
    // Create web server thread
    webServerThread_ = std::thread([this]() {
        // Simple HTTP server on port 8080
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "❌ Failed to create socket\n";
            return;
        }
        
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(8080);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "❌ Failed to bind to port 8080\n";
            close(server_fd);
            return;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "❌ Failed to listen on socket\n";
            close(server_fd);
            return;
        }
        
        webServerRunning_ = true;
        std::cout << "✓ Web server started on http://localhost:8080\n";
        std::cout << "🌐 Open http://localhost:8080 in your browser to view real-time visualization\n";
        
        // Accept connections and serve visualization page
        while (webServerRunning_ && !shouldStop_) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server_fd, &readfds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(server_fd + 1, &readfds, nullptr, nullptr, &timeout);
            
            if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
                int client_socket = accept(server_fd, nullptr, nullptr);
                if (client_socket >= 0) {
                    handleWebRequest(client_socket);
                    close(client_socket);
                }
            }
        }
        
        close(server_fd);
        std::cout << "✓ Web server stopped\n";
    });
}

void RealtimeVisualizer::broadcastDataToClients(const TrainingDataPoint& dataPoint) {
    if (!webServerRunning_) {
        return;
    }
    
    // Create JSON data
    std::ostringstream json;
    json << "{";
    json << "\"iteration\":" << dataPoint.iteration << ",";
    json << "\"mae\":" << std::scientific << std::setprecision(6) << dataPoint.meanAbsoluteError << ",";
    json << "\"sum\":" << std::setprecision(6) << dataPoint.utilitySum << ",";
    json << "\"time\":" << dataPoint.elapsedTime.count() << ",";
    json << "\"utilities\":[";
    for (size_t i = 0; i < dataPoint.playerUtilities.size(); ++i) {
        if (i > 0) json << ",";
        json << std::fixed << std::setprecision(6) << dataPoint.playerUtilities[i];
    }
    json << "]}";
    
    std::lock_guard<std::mutex> lock(latestDataMutex_);
    latestDataJson_ = json.str();
}

void RealtimeVisualizer::handleWebRequest(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);
    
    std::string request(buffer);
    std::string response;
    
    if (request.find("GET /data") == 0) {
        // Serve JSON data
        std::lock_guard<std::mutex> lock(latestDataMutex_);
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Cache-Control: no-cache\r\n";
        response += "\r\n";
        response += latestDataJson_;
    } else {
        // Serve HTML visualization page
        response = getVisualizationHTML();
    }
    
    send(client_socket, response.c_str(), response.length(), 0);
}

void RealtimeVisualizer::stopWebServer() {
    if (!webServerRunning_) {
        return;
    }
    
    webServerRunning_ = false;
    
    if (webServerThread_.joinable()) {
        webServerThread_.join();
    }
    
    std::cout << "✓ Web server stopped\n";
}

std::string RealtimeVisualizer::getVisualizationHTML() {
    return R"(HTTP/1.1 200 OK
Content-Type: text/html
Cache-Control: no-cache

<!DOCTYPE html>
<html>
<head>
    <title>Real-time MCCFR Training Visualization</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background-color: #0d1117; 
            color: #f0f6fc; 
            overflow-x: hidden;
        }
        h1 { 
            text-align: center; 
            color: #58a6ff; 
            margin: 15px 0;
            font-size: 2.2rem;
            font-weight: 600;
        }
        .container { 
            display: grid; 
            grid-template-columns: 2fr 2fr 1fr; 
            grid-template-rows: 1fr 1fr;
            gap: 15px; 
            height: calc(100vh - 80px);
            padding: 10px;
            max-width: 100vw;
        }
        .chart-container { 
            background: linear-gradient(135deg, #161b22 0%, #21262d 100%);
            padding: 15px; 
            border-radius: 12px; 
            border: 1px solid #30363d; 
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
            position: relative;
            overflow: hidden;
        }
        .chart-container::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, #58a6ff, #39d353, #f85149, #ffab40);
        }
        .chart-large {
            grid-row: span 2;
        }
        .stats { 
            background: linear-gradient(135deg, #161b22 0%, #21262d 100%);
            padding: 20px; 
            border-radius: 12px; 
            border: 1px solid #30363d;
            font-family: 'SF Mono', 'Monaco', 'Cascadia Code', 'Roboto Mono', monospace;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
            position: relative;
            overflow-y: auto;
        }
        .stats::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, #39d353, #58a6ff);
        }
        h2 { 
            margin: 0 0 15px 0; 
            color: #58a6ff; 
            font-size: 1.1rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .status { 
            font-size: 14px; 
            margin: 8px 0; 
            line-height: 1.4;
            padding: 2px 0;
        }
        .status-highlight {
            background: rgba(88, 166, 255, 0.1);
            padding: 8px 12px;
            border-radius: 6px;
            margin: 10px 0;
            border-left: 3px solid #58a6ff;
        }
        .zero-sum-ok { 
            color: #39d353; 
            font-weight: 600;
        }
        .zero-sum-warn { 
            color: #ff7b72; 
            font-weight: 600;
        }
        .metric-value {
            color: #79c0ff;
            font-weight: 600;
        }
        .player-utilities {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
            margin: 10px 0;
        }
        .player-util {
            background: rgba(48, 54, 61, 0.5);
            padding: 6px 10px;
            border-radius: 6px;
            font-size: 13px;
        }
        .player-0 { border-left: 3px solid #f85149; }
        .player-1 { border-left: 3px solid #58a6ff; }
        .player-2 { border-left: 3px solid #39d353; }
        .player-3 { border-left: 3px solid #ffab40; }
        
        /* Responsive design */
        @media (max-width: 1600px) {
            .container {
                grid-template-columns: 1fr 1fr;
                grid-template-rows: 1fr 1fr 0.8fr;
            }
            .chart-large {
                grid-row: span 1;
            }
            .stats {
                grid-column: span 2;
            }
        }
        
        @media (max-width: 1200px) {
            .container {
                grid-template-columns: 1fr;
                grid-template-rows: repeat(4, 1fr);
                height: auto;
                min-height: calc(100vh - 80px);
            }
            .chart-large {
                grid-row: span 1;
            }
            .stats {
                grid-column: span 1;
            }
            h1 {
                font-size: 1.8rem;
            }
        }
        
        /* Loading animation */
        .loading {
            display: inline-block;
            width: 12px;
            height: 12px;
            border: 2px solid #30363d;
            border-radius: 50%;
            border-top-color: #58a6ff;
            animation: spin 1s ease-in-out infinite;
        }
        
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <h1>🚀 Real-time MCCFR Training Visualization</h1>
    
    <div class="container">
        <div class="chart-container">
            <h2> Mean Absolute Error</h2>
            <canvas id="maeChart"></canvas>
        </div>
        
        <div class="chart-container">
            <h2> Zero-Sum Check</h2>
            <canvas id="sumChart"></canvas>
        </div>
        
        <div class= Player Utilities</h2>
            <canvas id="utilitiesChart"></canvas>
        </div>
        
        <div class="stats">
            <h2> Live Statistics</h2>
            <div id="stats">
                <div class="status">
                    <span class="loading"></span> Waiting for training data...
                </div>
            </div>
        </div>
    </div>

    <script>
        // Chart configurations
        const chartConfig = {
            type: 'line',
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index'
                },
                animation: {
                    duration: 0 // Disable animations for better performance
                },
                scales: {
                    x: { 
                        title: { 
                            display: true, 
                            text: 'Iteration',
                            color: '#f0f6fc',
                            font: { size: 14, weight: 'bold' }
                        },
                        grid: { 
                            color: '#30363d',
                            drawBorder: false
                        },
                        ticks: { 
                            color: '#8b949e',
                            font: { size: 12 }
                        }
                    },
                    y: { 
                        title: {
                            color: '#f0f6fc',
                            font: { size: 14, weight: 'bold' }
                        },
                        grid: { 
                            color: '#30363d',
                            drawBorder: false
                        },
                        ticks: { 
                            color: '#8b949e',
                            font: { size: 12 }
                        }
                    }
                },
                plugins: {
                    legend: { 
                        labels: { 
                            color: '#f0f6fc',
                            font: { size: 13, weight: '600' },
                            padding: 20,
                            usePointStyle: true,
                            pointStyle: 'circle'
                        }
                    },
                    tooltip: {
                        backgroundColor: 'rgba(13, 17, 23, 0.95)',
                        titleColor: '#f0f6fc',
                        bodyColor: '#f0f6fc',
                        borderColor: '#58a6ff',
                        borderWidth: 1,
                        cornerRadius: 8,
                        displayColors: true,
                        titleFont: { size: 14, weight: 'bold' },
                        bodyFont: { size: 13 }
                    }
                }
            }
        };

        // MAE Chart
        const maeChart = new Chart(document.getElementById('maeChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [{
                    label: 'Mean Absolute Error',
                    data: [],
                    borderColor: '#58a6ff',
                    backgroundColor: 'rgba(88, 166, 255, 0.1)',
                    fill: true,
                    borderWidth: 2,
                    pointRadius: 0,
                    pointHoverRadius: 4,
                    tension: 0.2
                }]
            },
            options: {
                ...chartConfig.options,
                scales: {
                    ...chartConfig.options.scales,
                    y: { 
                        ...chartConfig.options.scales.y,
                        type: 'logarithmic',
                        title: { 
                            display: true, 
                            text: 'MAE (log scale)',
                            color: '#f0f6fc',
                            font: { size: 14, weight: 'bold' }
                        }
                    }
                }
            }
        });

        // Zero-Sum Chart
        const sumChart = new Chart(document.getElementById('sumChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [{
                    label: '|Sum of Utilities|',
                    data: [],
                    borderColor: '#f85149',
                    backgroundColor: 'rgba(248, 81, 73, 0.1)',
                    fill: true,
                    borderWidth: 2,
                    pointRadius: 0,
                    pointHoverRadius: 4,
                    tension: 0.2
                }]
            },
            options: {
                ...chartConfig.options,
                scales: {
                    ...chartConfig.options.scales,
                    y: { 
                        ...chartConfig.options.scales.y,
                        type: 'logarithmic',
                        title: { 
                            display: true, 
                            text: '|Sum| (log scale)',
                            color: '#f0f6fc',
                            font: { size: 14, weight: 'bold' }
                        }
                    }
                }
            }
        });

        // Utilities Chart
        const utilitiesChart = new Chart(document.getElementById('utilitiesChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [
                    { 
                        label: 'SB (Small Blind)', 
                        data: [], 
                        borderColor: '#f85149', 
                        backgroundColor: 'rgba(248, 81, 73, 0.1)',
                        fill: false,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 4,
                        tension: 0.2
                    },
                    { 
                        label: 'BB (Big Blind)', 
                        data: [], 
                        borderColor: '#58a6ff', 
                        backgroundColor: 'rgba(88, 166, 255, 0.1)',
                        fill: false,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 4,
                        tension: 0.2
                    },
                    { 
                        label: 'CO (Cut Off)', 
                        data: [], 
                        borderColor: '#39d353', 
                        backgroundColor: 'rgba(57, 211, 83, 0.1)',
                        fill: false,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 4,
                        tension: 0.2
                    },
                    { 
                        label: 'BTN (Button)', 
                        data: [], 
                        borderColor: '#ffab40', 
                        backgroundColor: 'rgba(255, 171, 64, 0.1)',
                        fill: false,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 4,
                        tension: 0.2
                    }
                ]
            },
            options: {
                ...chartConfig.options,
                scales: {
                    ...chartConfig.options.scales,
                    y: { 
                        ...chartConfig.options.scales.y,
                        title: { 
                            display: true, 
                            text: 'Utility (Big Blinds)',
                            color: '#f0f6fc',
                            font: { size: 14, weight: 'bold' }
                        }
                    }
                }
            }
        });

        // Data fetching and updating
        let maxDataPoints = 1000;
        
        async function fetchData() {
            try {
                const response = await fetch('/data');
                const data = await response.json();
                updateCharts(data);
                updateStats(data);
            } catch (error) {
                console.log('Waiting for data...');
            }
        }

        function updateCharts(data) {
            const iteration = data.iteration;
            const mae = data.mae;
            const sum = Math.abs(data.sum);
            const utilities = data.utilities;

            // Update MAE chart
            maeChart.data.labels.push(iteration);
            maeChart.data.datasets[0].data.push(mae);
            
            // Update Sum chart
            sumChart.data.labels.push(iteration);
            sumChart.data.datasets[0].data.push(sum);
            
            // Update Utilities chart
            utilitiesChart.data.labels.push(iteration);
            for (let i = 0; i < 4; i++) {
                utilitiesChart.data.datasets[i].data.push(utilities[i]);
            }

            // Limit data points
            if (maeChart.data.labels.length > maxDataPoints) {
                maeChart.data.labels.shift();
                maeChart.data.datasets[0].data.shift();
                sumChart.data.labels.shift();
                sumChart.data.datasets[0].data.shift();
                utilitiesChart.data.labels.shift();
                for (let i = 0; i < 4; i++) {
                    utilitiesChart.data.datasets[i].data.shift();
                }
            }

            // Update charts
            maeChart.update('none');
            sumChart.update('none');
            utilitiesChart.update('none');
        }

        function updateStats(data) {
            const zeroSumOk = Math.abs(data.sum) < 1e-10;
            const statsDiv = document.getElementById('stats');
            
            // Format large numbers with commas
            const formatNumber = (num) => num.toLocaleString();
            
            statsDiv.innerHTML = `
                <div class="status-highlight">
                    <div class="status">Iteration: <span class="metric-value">${formatNumber(data.iteration)}</span></div>
                    <div class="status">MAE: <span class="metric-value">${data.mae.toExponential(3)}</span></div>
                    <div class="status">|Sum|: <span class="metric-value">${Math.abs(data.sum).toExponential(2)}</span></div>
                    <div class="status">Elapsed: <span class="metric-value">${formatNumber(data.time)}ms</span></div>
                </div>
                
                <div class="status" style="margin-top: 15px; margin-bottom: 10px; font-weight: 600;">Player Utilities:</div>
                <div class="player-utilities">
                    <div class="player-util player-0">
                        <strong>SB:</strong> ${data.utilities[0] >= 0 ? '+' : ''}${data.utilities[0].toFixed(4)}
                    </div>
                    <div class="player-util player-1">
                        <strong>BB:</strong> ${data.utilities[1] >= 0 ? '+' : ''}${data.utilities[1].toFixed(4)}
                    </div>
                    <div class="player-util player-2">
                        <strong>CO:</strong> ${data.utilities[2] >= 0 ? '+' : ''}${data.utilities[2].toFixed(4)}
                    </div>
                    <div class="player-util player-3">
                        <strong>BTN:</strong> ${data.utilities[3] >= 0 ? '+' : ''}${data.utilities[3].toFixed(4)}
                    </div>
                </div>
                
                <div class="status-highlight" style="margin-top: 15px;">
                    <div class="status ${zeroSumOk ? 'zero-sum-ok' : 'zero-sum-warn'}">
                        Zero-Sum Status: <strong>${zeroSumOk ? '✓ Perfect' : '✗ Warning'}</strong>
                    </div>
                    <div class="status" style="font-size: 12px; opacity: 0.8; margin-top: 5px;">
                        ${zeroSumOk ? 'Game theory constraints satisfied' : 'Sum deviates from zero - check game logic'}
                    </div>
                </div>
                
                <div class="status" style="margin-top: 15px; font-size: 12px; opacity: 0.6;">
                    Last updated: ${new Date().toLocaleTimeString()}
                </div>
            `;
        }

        // Start fetching data
        setInterval(fetchData, 100); // Update every 100ms
        fetchData(); // Initial fetch
    </script>
</body>
</html>)";
}

} // namespace mccfr
