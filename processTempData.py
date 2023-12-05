def process_temperature_data(file_path):
    temperatures = []

    # Open and read the data file
    with open(file_path, 'r') as file:
        # Skip the header line
        file.readline()

        # Read each line and extract the temperature value
        for line in file:
            _, temperature, *_ = line.strip().split(',')
            temperatures.append(float(temperature))

    # Calculate minimum, maximum, and average temperatures
    min_temp = temperatures[0]
    max_temp = temperatures[0]
    total_temp = 0

    for temp in temperatures:
        if temp < min_temp:
            min_temp = temp
        if temp > max_temp:
            max_temp = temp
        total_temp += temp

    average_temp = total_temp / len(temperatures)

    # Print the report
    print("Minimum Temperature:", min_temp)
    print("Maximum Temperature:", max_temp)
    print("Average Temperature:", average_temp)

process_temperature_data('myFeedData.csv')

