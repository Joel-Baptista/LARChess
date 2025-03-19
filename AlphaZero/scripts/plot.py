import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

# Set a Seaborn theme for aesthetics
sns.set_theme(style="whitegrid")

# Folder containing your CSV files
folder_path = '../models/slearn/model21'
# folder_path = '../models/alpha/model28'

# Iterate through each CSV file in the folder
for filename in os.listdir(folder_path):
    if filename.endswith('.csv'):
        # Read the CSV file
        file_path = os.path.join(folder_path, filename)
        df = pd.read_csv(file_path)
        
        # Automatically use the first column as the x-axis

        print(df[df.columns[1]])

        x_column = df.columns[0]

        # Melt the DataFrame to long-form for seaborn
        df_melted = pd.melt(df, id_vars=[x_column], var_name='Variable', value_name='Value')
        
        # Plotting using seaborn with enhanced styling
        plt.figure(figsize=(12, 8))
        sns.lineplot(x=x_column, y='Value', hue='Variable', data=df_melted, marker=' ', palette='tab10')

        # Customize the title and labels
        plt.title(f'{filename}', fontsize=16, weight='bold', color='darkblue', pad=20)
        plt.xlabel(x_column, fontsize=14, weight='bold', color='darkblue')
        plt.ylabel('Values', fontsize=14, weight='bold', color='darkblue')
        
        # Improve the legend
        plt.legend(title='Variables', title_fontsize='13', fontsize='11', loc='upper right', fancybox=True, framealpha=0.7)

        # Adjust the grid and spines for better aesthetics
        sns.despine(trim=True)
        plt.grid(True, which='both', linestyle='--', linewidth=0.5)

        # Save the plot as a PNG file (optional)
        output_filename = os.path.join(folder_path, f'{filename[:-4]}_plot.png')
        plt.savefig(output_filename, dpi=300, bbox_inches='tight')

        # Show the plot
        # plt.show()
