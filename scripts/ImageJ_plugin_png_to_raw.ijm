// Input folder that has one or more folders each with image slices
// This should be placed inside plugins subfolder of ImageJ

print("starting processing..");
// open stacks_byID
input = getDirectory("Input directory");
output = getDirectory("Output directory");

foldersList = getFileList(input);
// iterate over folders inside folder
for (i = 0; i < foldersList.length; i++) {
	filepath = input +  foldersList[i];
	if (File.isDirectory(filepath)) {
		stackToRaw(filepath, foldersList[i], output);
	} else {
		print("not dir: " + foldersList[i]);
	}
}

// for each, open the sequence image, then save as raw data
function stackToRaw(filepath, filename, output) {
	print("Processing: " + filepath);
	fileList = getFileList(filepath); 
	numberSlice=fileList.length;
	run("Image Sequence...", 
		  "open=[&filepath]"+
		  " number="+numberSlice+
		  " starting=1"+
		  " increment=1"+
		  " scale=100 "+
		  "file=[.png] "+
		  "sort");

	newname = substring(filename, 0, lengthOf(filename)-6) + ".raw"; 
	print(newname); 
	// save the image sequence as raw
	saveAs("Raw Data", output  + newname);
	close();
}
