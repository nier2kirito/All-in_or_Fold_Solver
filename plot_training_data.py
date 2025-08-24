#!/usr/bin/env python3
"""
Plot MCCFR training data to visualize improvement over time.
Usage: python plot_training_data.py [training_data.csv]
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def plot_training_data(csv_file='training_data.csv'):
    """Plot training data from CSV file."""
    
    if not os.path.exists(csv_file):
        print(f"Error: File {csv_file} not found!")
        return
    
    # Read the CSV data
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Error reading {csv_file}: {e}")
        return
    
    # Create subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
    fig.suptitle('MCCFR Training Progress', fontsize=16)
    
    # Plot 1: Mean Absolute Error over time
    ax1.plot(df['iteration'], df['mean_absolute_error'], 'b-', linewidth=2, marker='o', markersize=3)
    ax1.set_xlabel('Iteration')
    ax1.set_ylabel('Mean Absolute Error')
    ax1.set_title('Convergence: Mean Absolute Error')
    ax1.grid(True, alpha=0.3)
    ax1.set_yscale('log')  # Log scale for better visualization
    
    # Plot 2: Utility Sum (should be near zero)
    utility_sum_abs = np.abs(df['utility_sum'])
    ax2.plot(df['iteration'], utility_sum_abs, 'r-', linewidth=2, marker='o', markersize=3)
    ax2.set_xlabel('Iteration')
    ax2.set_ylabel('|Utility Sum|')
    ax2.set_title('Zero-Sum Property: |Sum of Utilities|')
    ax2.grid(True, alpha=0.3)
    ax2.set_yscale('log')  # Log scale for better visualization
    
    # Plot 3: Individual Player Utilities
    colors = ['red', 'blue', 'green', 'orange']
    labels = ['SB (Player 0)', 'BB (Player 1)', 'CO (Player 2)', 'BTN (Player 3)']
    
    for i in range(4):
        col = f'player_{i}_utility'
        if col in df.columns:
            ax3.plot(df['iteration'], df[col], color=colors[i], 
                    linewidth=2, marker='o', markersize=3, label=labels[i])
    
    ax3.set_xlabel('Iteration')
    ax3.set_ylabel('Utility')
    ax3.set_title('Player Utilities Over Time')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.axhline(y=0, color='black', linestyle='--', alpha=0.5)  # Zero line
    
    # Plot 4: Training Time
    if 'elapsed_time_ms' in df.columns:
        ax4.plot(df['iteration'], df['elapsed_time_ms'], 'purple', 
                linewidth=2, marker='o', markersize=3)
        ax4.set_xlabel('Iteration')
        ax4.set_ylabel('Elapsed Time (ms)')
        ax4.set_title('Training Time')
        ax4.grid(True, alpha=0.3)
    else:
        ax4.text(0.5, 0.5, 'No timing data available', 
                ha='center', va='center', transform=ax4.transAxes)
        ax4.set_title('Training Time (N/A)')
    
    plt.tight_layout()
    
    # Save the plot
    output_file = csv_file.replace('.csv', '_plot.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Plot saved to: {output_file}")
    
    # Show the plot
    plt.show()
    
    # Print summary statistics
    print("\n=== Training Summary ===")
    print(f"Total iterations: {df['iteration'].max()}")
    print(f"Final mean absolute error: {df['mean_absolute_error'].iloc[-1]:.6f}")
    print(f"Final utility sum: {df['utility_sum'].iloc[-1]:.2e}")
    print(f"Total training time: {df['elapsed_time_ms'].iloc[-1]} ms")
    
    print("\nFinal player utilities:")
    for i in range(4):
        col = f'player_{i}_utility'
        if col in df.columns:
            print(f"  {labels[i]}: {df[col].iloc[-1]:.6f}")

if __name__ == "__main__":
    csv_file = sys.argv[1] if len(sys.argv) > 1 else 'training_data.csv'
    plot_training_data(csv_file)
