Readme - Low level Char driver
==============================

Concept
-------
	A character (char) device is one that can be accessed as a stream of bytes (like a file); a char driver is
in charge of implementing this behaviour. Such a driver usually implements at least the open, close, read, and
write system calls. The text console (/dev/console) and the serial ports (/dev/ttyS0 and friends) are examples
of char devices, as they are well represented by the stream abstraction. The only relevant difference between a
char device and a regular file is that you can always move back and forth in the regular file, whereas most 
char devices are just data channels, which you can only access sequentially.

Syntax & Semantics
------------------
	At the base of the char driver, lies the structure 'struct cdev'. Initially declare a variable of such type :
	struct cdev mycdev;
	
	Before proceeding any further, we need to get the device numbers for the device and register the driver :
		alloc_chrdev_region(&deviceno , minor, NMINORS , DRIVER_NAME);
		
	The first argument is of type 'dev_t' which can hold the device number. The second argument is the first
minor number of the device, the third is the number of minors required and the last argument is the name of the
driver.

	Now define the file_operations structure for the devices :
	struct file_operations chardriver_lowlevel_fops= {
		.owner 			= THIS_MODULE,
		.open			= chardriver_lowlevel_open,
		.release		= chardriver_lowlevel_release,
		.read			= chardriver_lowlevel_read,
		.write			= chardriver_lowlevel_write,
		.unlocked_ioctl = chardriver_lowlevel_ioctl
	};
	
	Now add the initialise the devices and add it to the driver :
		cdev_init(&mycdev , &chardriver_lowlevel_fops);
	
	The first argument specifies the 'struct cdev' of this driver and the second argument is the associated
file operations
	
		cdev_add(&mycdev, deviceno, 1);
	The first argument specifies the 'struct cdev' of this driver, the second argument is the device number to 
be added and the third argument is the no. of devices to be added.

	The char device is now live and the file operations can be performed on it :
	int chardriver_lowlevel_open(struct inode *inod, struct file *filp)
	{
		/* Your code here */
	}
		The open function is called when open() is called on the device. All the initialisation at the time
	of opening the device are to be done in this function.
	
	int chardriver_lowlevel_release(struct inode *inod, struct file *filp)
	{
		/* Your code here */
	}
	
		The release function is called when close() is called on the device. All the cleanup at the time of
	closing the device are to be done in this function.
	
	ssize_t chardriver_lowlevel_read(struct file *filp, char __user *Ubuff, size_t count, loff_t *offp)
	{
		/* Your code here */
		copy_to_user();
	}
		The read function is called when read() is called on the device. The function is used to send data to the
	user space by a call to copy_to_user(). The function must return the no. of bytes actually written to the
	user-space.
	
	ssize_t chardriver_lowlevel_write(struct file *filp, const char __user *Ubuff,
										size_t count, loff_t *offp)
	{
		/* Your code here */
		copy_from_user();
	}
		The write function is called when write() is called on the device. The function is used to get the data
	from the user-space by a call to copy_from_user(). The function must return the no. of bytes actually read
	from the user-space.
	
	long chardriver_lowlevel_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
	{
		/* Your code here */		
	} 
		The unlocked_ioctl is called when ioctl() is called on the device. The function is used to change the
	device's configuration at run-time. The implementation of ioctl is usually a switch statement based on the
	command number. On a successful command call, '0' must be returned, else '-EINVAL' or '-ENOTTY' has to be
	returned.
	
	After the operations on the char device are done, the device can be removed from the driver and finally the
driver can be unregistered.
	cdev_del(&mycdev);
	unregister_chrdev_region(deviceno, NMINORS);
	
	
		
	
	
	

		
