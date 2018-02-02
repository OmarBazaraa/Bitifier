# Introduction
For many decades, human beings used to store their valuable information, data and records as a written text on different kind of papers.
Then they invented the computers and used them to store data which its size continued to increase rapidly as the time passes, which enforced people to ﬁnd proper ways to minimize the storage required to hold data.
To satisfy such need, computer scientists and engineers made too many attempts to compress the data and increase the compression ratio.

Computer scientists and engineers later realized that the big share of information that is required to be compressed is plain text (in archives, libraries, universities, …etc).
With that being noticed, the bi-level compression algorithms (e.g. JBIG2) development took an increasing exponential improvement curve.
In this report, we’ll be discussing the lossless algorithm that we’ve developed to compress text images with a high compression ratio (**523** on a provided data set), **Bitiﬁer** was inspired by some techniques and other algorithms which we’re going to mention later on

# Bitiﬁer History
**Bitiﬁer** has come through many stages to reach what it is today, in the following paragraphs and graph we will be showing the development history and how each added technique/algorithm contributed in shaping the current Bitiﬁer.
![alt text](https://lh3.googleusercontent.com/E_BFzBojQ979jlv-PeAy04YlL-iax1ECs0aAfZEDhQuNaYCmD-yCy_oSCF8kx_K3pEMigET8hyYrhK9TNKZz=w1920-h949-rw)

# Bitifier
**Bitiﬁer** is a compression algorithm that takes the input ﬁle and passes it through many layers to get the compressed result. The following list shows those layers in a simple way.

### Colored Image to bi-level conversion:
During this process the input image is being read using OpenCV (C++ version) and then it gets converted to gray scale image (255 levels), after that we’re converting the image to bi-level one using a threshold (gray level > 180 = white, black otherwise).

### Characters detection:
Taking advantage of the fact that we’re actually compressing text images, we detect the characters in the given text image (letters, numbers, symbols, …etc) using simple DFS algorithm to get the boundaries surrounding each character. Then for each distinct character we store the width and the height of the its surrounding rectangle and then we try different run-length encoding techniques (horizontal, vertical, spiral & zig-zag) and output the encoded data having the minimum number of bytes along with a unique id representing the used technique.
We then store the occurrences positions — in the image —  along with a unique id referring to one of the encoded distinct characters.
To reduce data size, the character occurrences positions are stored in a relative way, which means that the actual stored position is the difference between this occurrence and the previous one.

### Byte concatenation:
This technique takes the output from the previous stage (integers) and removes any null bytes and store some meta data to restore those integers when decoding.
The basic idea lies behind the fact that most integers come with the 2 most signiﬁcant bytes are null, so we remove those null values and store meta data that tells us how many non-null bytes exist in this integer (then we can determine how many null bytes were originally there by this simple calculation: 4 - non-null bytes count). By following this method we were able to replace a lot of integer 4 bytes by a 2 bytes.
After that we’ve noticed that the meta data related to this part can be encoded using run length encoding and now we can represent 64 bytes —maximum— of the meta data in one single byte.

In this example we will show what this layer is capable of doing:

Let’s assume that we have the following integers as input to this layer (each digit is a single byte):

`0001 0002 0003 0004 0045`

As you can see there’re a lot of null characters (0 byte) in the above input, and the input total size is 4*5 = 20 bytes.
By taking the ﬁrst integer, it has 3 null bytes so we need to store in the meta data that it has only one data byte (one data byte is represented by 00 and 2 data bytes is represented as 01 and so on, so we can use only 2 bits to know the number of non-null bytes in the given integer), and as you can see that we have 4 successive integers with only one non-null byte, now the data the really needs to be encoded will be 

`123445`

and the meta data will have 2 bytes 

* `00000100`

most signiﬁcant 2 bits means that the integers contain only one non-null byte, then the last 6 bits contains the length of the run (in this case 4 successive integers)
and

* `01000001`

most signiﬁcant 2 bits means that the integers contain only 2 non-null bytes, then the last 6 bits contains the length of the run (in this case 1 integer).

As you can see the total output bytes count 6 bytes (data) + 2 bytes (meta-data) = 8 bytes instead of 20 original ones, and we can assure you that as the data gets larger, this ratio gets better and better. 

### Huffman:
This last layer uses the basic Huffman algorithm to encode the data from previous layers, and after encoding the frequencies of the symbols (needed to rebuild the Huffman tree when decoding) we use the bit concatenation (same as byte concatenation but on the bit level not only bytes) to reduce the size.

# Encoded File Content
![alt text](https://lh5.googleusercontent.com/97Fks8XnDRZKeA480qXjv-HJjYvWHgqcnfnUDUdDPyoCw7_4naYPQ1OL-jXfv7gXD-faUpzOGzNK9CjphHIH=w1920-h949-rw)

# Decoding
**Bitiﬁer** decoding process is just a simple inverse to the above described technique and it takes the “ﬁnal output ﬁle” from the above diagram as its input. We ﬁrst start by decoding the Huffman encoded string into some sort of symbols taking the advantage of the fact that Huffman is an instantaneous code, after that we’ve to invert the byte concatenation algorithm effect by using its inverse method, for example if we’ve to decode the data in the above example (123445) and the corresponding meta data (00000100 01000001), the ﬁrst byte in the meta data tells us that the ﬁrst 4 bytes of the data were originally integers with only one (00) non-null byte, so we can expand the ﬁrst 4 bytes to (0001 0002 0003 0004), then the second byte in the meta data tells us that the next 2 bytes were originally integers with only two (01) non-null bytes, so we can expand the next two bytes to (0045). As you can see we were able to retrieve the original data.
The main concern while applying the inverse byte concatenation technique is distinguishing the actual data from the meta data, as we don’t put any separators. We simply start reading the ﬁle from both ends and while reading the meta data from the ﬁle second end we accumulate the data size (as the data size is stored in the meta data) and at some point when the data (which is read from the ﬁrst end of the document) equals the 
accumulated data size (calculated from meta data), we now know that we’ve decoded all the data.
Finally, we start with a white image and start putting the characters in the correct positions which are speciﬁed in the encoded ﬁle meta data.

# Other Algorithms
We’ve been trying to integrate other algorithms for a while, but the ratio wasn’t improving so far. However, we’ll mention those trials in the following list.

* **LZW**:
We’ve implemented LZW algorithm and tried it after some layers but the results were always negative, due to the rapid increase in LZW indices lengths.

* **Bit concatenation**:
The bit concatenation method is implemented the same way as mentioned in the byte concatenation method, but on the bit level not only bytes. This method is really efﬁcient and it produced a great ratio. However, it cannot be used before Huffman, as Huffman takes advantage of repeated data (which may be the zeros we’re eliminating when using this algorithm). So, the overall ratio (without Huffman) was ~400. Then we decided to go with byte concatenation plus Huffman which produced compression ratio ~523.

# Conclusion
We deeply believe that this algorithm can be improved much more than its current state. After some researches we’ve found that if we need to break through the current ratio barrier, we’ll need some heavy mathematics models. We still have some simple ideas to try later such as storing the text baselines and store for each letter its baseline and its x offset relative to previous letter on the same baseline and y offset relative to the baseline origin y. We may continue developing this algorithm as the power of hope pushes us further to improve and enhance this bi-level lossless images compression algorithm. We also welcome any kind of recommendations or contributions to this algorithm. 
