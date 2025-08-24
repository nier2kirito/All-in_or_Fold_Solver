/**
 * @file realtime_visualizer.hpp
 * @brief Direct real-time visualization for MCCFR training without intermediate files
 */

#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <chrono>

namespace mccfr {

/**
 * @brief Training data point for real-time visualization
 */
struct TrainingDataPoint {
    int iteration;
    double meanAbsoluteError;
    double utilitySum;
    std::vector<double> playerUtilities;
    std::chrono::milliseconds elapsedTime;
    
    TrainingDataPoint(int iter, double mae, double sum, 
                     const std::vector<double>& utilities, 
                     std::chrono::milliseconds elapsed)
        : iteration(iter), meanAbsoluteError(mae), utilitySum(sum), 
          playerUtilities(utilities), elapsedTime(elapsed) {}
};

/**
 * @brief Real-time data streamer for MCCFR training visualization
 * 
 * This class provides direct, live streaming of training data without
 * intermediate files. It uses a separate thread to handle visualization
 * while training continues uninterrupted.
 */
class RealtimeVisualizer {
public:
    /**
     * @brief Configuration for real-time visualization
     */
    struct Config {
        bool enabled = false;                    ///< Enable real-time visualization
        int updateInterval = 10;                 ///< Update every N iterations
        int maxDataPoints = 1000;                ///< Maximum data points to keep
        bool showConsoleStats = true;            ///< Show stats in console
        std::string windowTitle = "MCCFR Training"; ///< Visualization window title
    };

    /**
     * @brief Constructor
     */
    RealtimeVisualizer();
    explicit RealtimeVisualizer(const Config& config);
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~RealtimeVisualizer();
    
    /**
     * @brief Start the real-time visualization system
     * @return true if successfully started
     */
    bool start();
    
    /**
     * @brief Stop the visualization system
     */
    void stop();
    
    /**
     * @brief Add a new data point for visualization
     * @param dataPoint The training data point to visualize
     */
    void addDataPoint(const TrainingDataPoint& dataPoint);
    
    /**
     * @brief Check if visualization is running
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Get current configuration
     */
    const Config& getConfig() const { return config_; }

private:
    Config config_;
    
    // Threading
    std::atomic<bool> running_{false};
    std::atomic<bool> shouldStop_{false};
    std::thread visualizationThread_;
    std::mutex dataMutex_;
    std::condition_variable dataCondition_;
    
    // Data storage
    std::deque<TrainingDataPoint> dataPoints_;
    
    // Visualization methods
    void visualizationLoop();
    void updatePlots();
    void printConsoleStats(const TrainingDataPoint& latest);
    
    // Web server for localhost visualization
    void startWebServer();
    void stopWebServer();
    void handleWebRequest(int client_socket);
    void broadcastDataToClients(const TrainingDataPoint& dataPoint);
    std::string getVisualizationHTML();
    
    // Web server management
    std::atomic<bool> webServerRunning_{false};
    std::thread webServerThread_;
    std::mutex latestDataMutex_;
    std::string latestDataJson_;
};

} // namespace mccfr
