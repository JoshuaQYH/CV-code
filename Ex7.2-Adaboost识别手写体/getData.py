from PIL import Image
import numpy as np
 
def img2vector(label, filename):
    img = Image.open(filename)
    img_ndarray = np.asarray(img, dtype = 'int')
    returnVect = np.ndarray.flatten(img_ndarray)
    return returnVect


csv_data = []
for i in range(10):
	filename = str(i) + ".bmp"
	sample = []
	sample.append(i)
	data = img2vector(i, filename)
	data = data.tolist()
	csv_data.append(sample + data)
print (csv_data)
np.savetxt('digit.csv', csv_data, delimiter=',')