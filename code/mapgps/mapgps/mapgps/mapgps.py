
import gmplot
import csv

def read_gps_data_from_csv(file_path):
    latitude_list = []
    longitude_list = []

    with open(file_path, mode='r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            if row[0] == '2':
                try:
                    latitude = float(row[2])
                    longitude = float(row[3])
                    latitude_list.append(latitude)
                    longitude_list.append(longitude)
                except ValueError:
                    # Skip rows with invalid data
                    continue

    return latitude_list, longitude_list

# Path to your CSV file
file_path = "C:\\source\\fairchildlabs\\event.txt"

# Read GPS data from CSV file
latitude_list, longitude_list = read_gps_data_from_csv(file_path)

# Create a plot on the map
if latitude_list and longitude_list:
    gmap = gmplot.GoogleMapPlotter(latitude_list[0], longitude_list[0], 15)  # Start plotting at the first point
    gmap.scatter(latitude_list, longitude_list, color='red', size=50, marker=True)

    # Save the map to an HTML file
    gmap.draw("map.html")
else:
    print("No valid GPS data found in the CSV file.")
