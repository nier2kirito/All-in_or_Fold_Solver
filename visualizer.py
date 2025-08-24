import matplotlib.pyplot as plt
import numpy as np
import re
import argparse
import os

def parse_infosets(file_path):
    """Parse MCCFR strategy file and extract hand data"""
    hand_data = {}
    
    with open(file_path, 'r') as file:
        lines = file.readlines()
        
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Skip comments and empty lines
        if line.startswith('#') or not line:
            i += 1
            continue
            
        # Parse InfoSet line
        if line.startswith('InfoSet:'):
            # Extract information from InfoSet line
            # Format: InfoSet: P2:[P0:P][P1:P]75o Pot:1.4 Visits: 1885
            info_match = re.match(r'InfoSet:\s+(.+)\s+Pot:([0-9.]+)\s+Visits:\s+(\d+)', line)
            if info_match:
                infoset_str = info_match.group(1)
                pot_size = float(info_match.group(2))
                visits = int(info_match.group(3))
                
                # Parse position and hand from infoset string
                # Format: P2:[P0:P][P1:P]75o or P2:[P0:P][P1:P]Q10o
                position_match = re.match(r'P(\d+):', infoset_str)
                # Extract hand from the end of the string after the last ']'
                last_bracket = infoset_str.rfind(']')
                if last_bracket != -1:
                    hand = infoset_str[last_bracket + 1:].strip()
                else:
                    hand = None
                
                if position_match and hand:
                    position = f"P{position_match.group(1)}"
                    
                    # Get strategy line
                    if i + 1 < len(lines):
                        strategy_line = lines[i + 1].strip()
                        if strategy_line.startswith('Strategy:'):
                            strategy_str = strategy_line.replace('Strategy:', '').strip()
                            probabilities = [float(p) for p in strategy_str.split()]
                            
                            if len(probabilities) >= 2:
                                fold_prob = probabilities[0]
                                all_in_prob = probabilities[1]
                                
                                # Use position as the decision key
                                hand_data[(position, hand)] = (fold_prob, all_in_prob)
                                
                            i += 2  # Skip strategy line
                            continue
                            
        i += 1
    
    return hand_data

# Function to map hands to matrix position
def hand_to_position(hand):
    """Convert hand string like 'AKo', 'AKs', 'AA', '1010', 'Q10o' to matrix position"""
    rank_map = {'2': 0, '3': 1, '4': 2, '5': 3, '6': 4, '7': 5, '8': 6, '9': 7, '10': 8, 'T': 8, 'J': 9, 'Q': 10, 'K': 11, 'A': 12}
    
    try:
        # Parse the hand string
        if hand.endswith('o') or hand.endswith('s'):
            # Suited or offsuit hand
            suit_indicator = hand[-1]
            hand_part = hand[:-1]
            is_suited = suit_indicator.lower() == 's'
        else:
            # Pocket pair
            hand_part = hand
            is_suited = False
        
        # Extract ranks
        if '10' in hand_part:
            # Handle hands with 10
            if hand_part.startswith('10'):
                rank1 = '10'
                rank2 = hand_part[2:]
            elif hand_part.endswith('10'):
                rank1 = hand_part[:-2]
                rank2 = '10'
            elif '10' in hand_part[1:]:
                rank1 = hand_part[0]
                rank2 = '10'
            else:
                return None
        else:
            # Regular hands without 10
            if len(hand_part) == 2:
                rank1, rank2 = hand_part[0], hand_part[1]
            else:
                return None
        
        # Get rank indices
        if rank1 not in rank_map or rank2 not in rank_map:
            return None
            
        rank1_value = rank_map[rank1]
        rank2_value = rank_map[rank2]
        
        # For pocket pairs, return diagonal position
        if rank1_value == rank2_value:
            return (rank1_value, rank2_value)
        
        # Return position for suited (above diagonal) or offsuit (below diagonal)
        if is_suited:
            return (min(rank1_value, rank2_value), max(rank1_value, rank2_value))
        else:
            return (max(rank1_value, rank2_value), min(rank1_value, rank2_value))
    
    except (ValueError, IndexError, KeyError):
        # In case of invalid hands, return None
        return None

# Function to create the hand range plot
def plot_hand_range(hand_data, position_filter=None, output_file=None):
    """Create and save hand range plot"""
    # Initialize a 13x13 matrix for the plot
    fold_matrix = np.zeros((13, 13))
    all_in_matrix = np.zeros((13, 13))
    
    # Populate the matrices with fold and all-in probabilities
    for (position, hand), (fold_prob, all_in_prob) in hand_data.items():
        # Filter by position if specified
        if position_filter and position != position_filter:
            continue
            
        pos = hand_to_position(hand)
        
        if pos:
            row, col = pos
            fold_matrix[row, col] = fold_prob
            all_in_matrix[row, col] = all_in_prob
    
    # Create the plot
    fig, ax = plt.subplots(figsize=(12, 10))
    
    # Card ranks for labels (reversed order from A to 2)
    display_ranks = ['A', 'K', 'Q', 'J', '10', '9', '8', '7', '6', '5', '4', '3', '2']
    # Matrix indices for each rank
    rank_to_index = {'2': 0, '3': 1, '4': 2, '5': 3, '6': 4, '7': 5, '8': 6, '9': 7, '10': 8, 'J': 9, 'Q': 10, 'K': 11, 'A': 12}
    
    # Define clearer blue and more reddish red
    fold_color = '#0066CC'  # Clearer blue
    all_in_color = '#FF3333'  # More reddish red
    
    # Create a grid for the rectangles
    for i in range(13):
        for j in range(13):
            # Get the actual ranks based on the axis ordering
            row_rank = display_ranks[12-i]  # Y-axis is 2 to A (top to bottom) - inverted
            col_rank = display_ranks[j]     # X-axis is A to 2 (left to right)
            
            # Get the matrix indices for these ranks
            row_idx = rank_to_index[row_rank]
            col_idx = rank_to_index[col_rank]
                
            # Get the fold and all-in probabilities
            fold_width = fold_matrix[row_idx, col_idx]
            all_in_width = all_in_matrix[row_idx, col_idx]
            
            # Determine if suited or offsuit based on position relative to diagonal
            if row_idx < col_idx:  # Above diagonal (suited)
                hand_label = f"{col_rank}{row_rank}s"
            elif row_idx > col_idx:  # Below diagonal (offsuit)
                hand_label = f"{col_rank}{row_rank}o"
            else:  # Diagonal (pairs)
                hand_label = f"{col_rank}{row_rank}"
            
            # Normalize if needed
            total = fold_width + all_in_width
            if total > 0:
                fold_width = fold_width / total
                all_in_width = all_in_width / total
                
                # Create rectangles for each probability
                # Fold (blue) on the left side
                rect_fold = plt.Rectangle((j, i), fold_width, 1, color=fold_color)
                # All-in (red) on the right side
                rect_all_in = plt.Rectangle((j + fold_width, i), all_in_width, 1, color=all_in_color)
                
                # Add rectangles to the plot
                ax.add_patch(rect_fold)
                ax.add_patch(rect_all_in)
            
            # Add hand label to the upper left corner of each square
            ax.text(j + 0.05, i + 0.95, hand_label, 
                    ha='left', va='top', 
                    fontsize=8, color='white',
                    fontweight='bold', fontfamily='serif')
    
    # Set axis limits
    ax.set_xlim(0, 13)
    ax.set_ylim(0, 13)
    
    # Set axis labels
    ax.set_xticks(np.arange(13) + 0.5)
    ax.set_yticks(np.arange(13) + 0.5)
    ax.set_xticklabels(display_ranks)  # X-axis: A to 2 (left to right)
    ax.set_yticklabels(list(reversed(display_ranks)))  # Y-axis: 2 to A (top to bottom)
    
    # Move x-axis labels to the top
    ax.xaxis.tick_top()
    ax.xaxis.set_label_position('top')
    
    # Add grid lines only at rank boundaries
    ax.set_xticks(np.arange(14), minor=True)
    ax.set_yticks(np.arange(14), minor=True)
    ax.grid(which='minor', color='black', linestyle='-', linewidth=0.5)
    
    # Remove tick marks
    ax.tick_params(which='both', length=0)
    
    # Add a legend
    fold_patch = plt.Rectangle((0, 0), 1, 1, color=fold_color, label='Fold')
    all_in_patch = plt.Rectangle((0, 0), 1, 1, color=all_in_color, label='All-in')
    ax.legend(handles=[fold_patch, all_in_patch], loc='lower right')
    
    # Add title with a nicer font
    title = f'Poker Hand Strategy Visualization'
    if position_filter:
        title += f' - {position_filter}'
    ax.set_title(title, fontfamily='serif', fontsize=14, pad=20)
    
    # Save or display the plot
    plt.tight_layout()
    
    if output_file:
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Chart saved to {output_file}")
        plt.close()  # Close the figure to free memory
    else:
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Visualize MCCFR Strategy as GTO Wizard-style charts')
    parser.add_argument('strategy_file', help='Path to the MCCFR strategy file')
    parser.add_argument('--position', '-p', help='Player position to visualize (e.g., P0, P1, P2, P3)')
    parser.add_argument('--output', '-o', help='Output file path (PNG format)')
    parser.add_argument('--output-dir', '-d', help='Output directory for multiple charts')
    parser.add_argument('--all-positions', '-a', action='store_true', 
                       help='Generate charts for all positions')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.strategy_file):
        print(f"Error: Strategy file '{args.strategy_file}' not found")
        return
    
    # Parse the strategy file
    print(f"Parsing strategy file: {args.strategy_file}")
    hand_data = parse_infosets(args.strategy_file)
    
    if not hand_data:
        print("No data found in the strategy file")
        return
    
    # Get available positions
    positions = set(position for position, hand in hand_data.keys())
    print(f"Available positions: {sorted(positions)}")
    
    if args.all_positions:
        # Generate charts for all positions
        if args.output_dir:
            os.makedirs(args.output_dir, exist_ok=True)
            
        for position in sorted(positions):
            output_file = None
            if args.output_dir:
                output_file = os.path.join(args.output_dir, f'strategy_{position}.png')
            elif args.output:
                base, ext = os.path.splitext(args.output)
                output_file = f"{base}_{position}{ext}"
                
            plot_hand_range(hand_data, position, output_file)
    else:
        # Generate chart for specific position or first available
        position = args.position
        if not position:
            position = sorted(positions)[0]
            print(f"No position specified, using: {position}")
        elif position not in positions:
            print(f"Position {position} not found. Available: {sorted(positions)}")
            return
            
        output_file = args.output
        if not output_file:
            output_file = f'strategy_{position}.png'
            
        plot_hand_range(hand_data, position, output_file)

if __name__ == '__main__':
    main()
