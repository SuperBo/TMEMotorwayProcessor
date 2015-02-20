# Introduction
This is a small data processor for the TME Motorway Dataset which is aimed at object detection and tracking purposes. The data processor reads out the images, converts them to RGB and also processes the bounding boxes from the ground truth file provided. The dataset can be downloaded from: http://cmp.felk.cvut.cz/data/motorway/

# TME Motorway Dataset
The “Toyota Motor Europe (TME) Motorway Dataset” is composed by 28 clips for a total of approximately 27 minutes (30000+ frames) with vehicle annotation. Annotation was semi-automatically generated using laser-scanner data. Image sequences were selected from acquisition made in North Italian motorways in December 2011. This selection includes variable traffic situations, number of lanes, road curvature, and lighting, covering most of the conditions present in the complete acquisition. 

# Dependencies
- OpenCV 3.0.0
- Boost 1.56.0

# Usage
You have to download the TME Motorway Dataset yourself (http://cmp.felk.cvut.cz/data/motorway/). The first 50 images are provided in this repository for testing purposes. However you need to add the rest of the images to the correct paths yourself. There are seperate directories for the DAYLIGHT and SUNSET video sequences and the ground truth annotations are also splitted into these two subsets. After downloading the TME Motorway Dataset you can convert all the images from BayerRB to RGB images using the `convertImagesToRGB()` method.

# Credits
Many thanks to Claudio Caraffi (and others) for creating the TME Motorway Dataset. Also the conversion of screen coordinates to world coordinates is mainly copied from the evaluation code they provide. 


