Kernel module for the National Instruments DAQCard-4050 PCMCIA Digital Multimeter card.

The code is written according to the MHDDK for Windows CE example provided by the NI here:
https://lumen.ni.com/nicif/us/evalmhddk/content.xhtml

I have tested all of the measurement modes, and works fine.

The code can compiled under 2.6.38, and probably on the most modern kernels (2.6=<). (I have verified only under it.)

The module requires _**no NI software**_ to be installed.

The API is very simple, an example graphical frontend is provided written using [Qt4](http://qt.nokia.com/products), and [Qwt](http://qwt.sourceforge.net):

![http://users.atw.hu/balubati/blog/images/nidmm.png](http://users.atw.hu/balubati/blog/images/nidmm.png)