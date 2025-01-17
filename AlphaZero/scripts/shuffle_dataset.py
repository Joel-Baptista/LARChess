import pandas as pd
import os
import random

def join_and_shuffle_csv(input_file1, input_file2, output_file):
    """Load two CSV files, join them, shuffle the rows, and save as one file."""
    # Check if the files exist
    if not os.path.exists(input_file1):
        print(f"Error: File '{input_file1}' does not exist.")
        return

    if not os.path.exists(input_file2):
        print(f"Error: File '{input_file2}' does not exist.")
        return

    # Load the CSV files
    try:
        df1 = pd.read_csv(input_file1)
        df2 = pd.read_csv(input_file2)
    except Exception as e:
        print(f"Error reading files: {e}")
        return

    # Join the DataFrames
    combined_df = pd.concat([df1, df2], ignore_index=True)

    # Shuffle the combined DataFrame rows
    combined_df = combined_df.sample(frac=1, random_state=random.randint(0, 1000)).reset_index(drop=True)
    
    combined_df = combined_df.loc[:, ~combined_df.columns.str.contains('^Unnamed')]

    # Save the shuffled DataFrame to the output file
    try:
        combined_df.to_csv(output_file, index=False)
        print(f"Shuffled combined CSV saved to '{output_file}'.")
    except Exception as e:
        print(f"Error writing to file '{output_file}': {e}")

if __name__ == "__main__":

    join_and_shuffle_csv("../datasets/games_processed_v2.csv","../datasets/processed_tactic_evals.csv","../datasets/dataset.csv")
