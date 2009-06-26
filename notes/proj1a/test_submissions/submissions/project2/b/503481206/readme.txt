Name: Richard Ho
UID: 003471455
Name: Gene Auyeung
UID: 503481206

In Project 2b, we ran into a small error that we are unsure about.  When we try to write with value 0 at buffer[0] we get a segmentation error, but if we change it to 1, then it works.  We're not sure whether this is a bug in pagefile or is it something that we have to do to work around it.  Because pid is valid at 0, then we feel like buffer[0] (which we set to be pid of RecordId) should be able to have the value of 0 without any errors.
