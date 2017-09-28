/*
 ========================================================================================
 Driver Name : led74hc595
 Author      : ThinhNguyen
 License	 : GPL
 Description : LED 7seg 74HC595. hshop.vn
 ========================================================================================
 */

#include"led74hc595.h"

#include"led74hc595_ioctl.h"

#define SIZE 									1024
#define NMINORS 								1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ThinhNguyen");

/* Define a device specific data */
typedef struct privatedata {
	int nMinor;
	char Kbuff[SIZE];
	struct cdev led;

} led74hc595_private;

led74hc595_private devices[NMINORS];
struct hrtimer htimer;
static ktime_t kt_period;

/* Declare the required variables */
int major;
int minor = 1;
dev_t deviceno;
struct class *led74hc595_class;
struct device *led74hc595_device;

/* Char driver open function : Called when open() is called on the device */
int led74hc595_open(struct inode *inod, struct file *filp)
{
	/* Assign the address of device specific structure into filp->private_data
	 * such that it can be accessed across all the functions, each pertaining to
	 * the specific device
	 */
	led74hc595_private *dev = container_of(inod->i_cdev,
			led74hc595_private, led);
	filp->private_data = dev;

	PINFO("In char driver open() function device node : %d\n", dev->nMinor);

	return 0;
}

/* Char driver release function : Called when close() is called on the device */
int led74hc595_release(struct inode *inod, struct file *filp)
{
	/* Retrieve the device specific structure */
	led74hc595_private *dev = filp->private_data;

	PINFO("In char driver release() function device node : %d\n", dev->nMinor);
	return 0;
}

/* Char driver read function : Called when read() is called on the device
 * It is used to copy data to the user space.
 * The function must return the number of bytes actually transferred
 */
ssize_t led74hc595_read(struct file *filp, char __user *Ubuff, size_t count, loff_t *offp)
{
	/* Retrieve the device specific structure */
	led74hc595_private *dev = filp->private_data;
	int res;

	PINFO("In char driver read() function\n");

	/* copy the data from kernel buffer to User-space buffer */
	res = copy_to_user((char *)Ubuff , (char *)dev->Kbuff , strlen(dev->Kbuff) + 1);
	if(res == 0)
	{
		PINFO("data from kernel buffer to user buffer copied successfully with bytes : %d\n",
				strlen(dev->Kbuff));
		return strlen(dev->Kbuff);

	} else {

		printk("copy from kernel to user failed\n");
		return -EFAULT;
	}

	return 0;
}

/* Char driver read function : Called when read() is called on the device
 * It is used to copy data to the user space.
 * The function must return the number of bytes actually transferred
 */
ssize_t led74hc595_write(struct file *filp, const char __user *Ubuff,
		size_t count, loff_t *offp)
{
	/* Retrieve the device specific structure */
	led74hc595_private *dev = filp->private_data;
	int res;

	PINFO("In char driver write() function\n");

	/* Copy data from user space buffer to driver buffer */
	memset(dev->Kbuff,0,sizeof(dev->Kbuff));
	res = copy_from_user((char *)dev->Kbuff , (char *) Ubuff,count);
	if(res == 0)
	{
		PINFO("data from the user space : %s no of bytes : %d\n",dev->Kbuff, count);
		return count;
	} else {

		PERR("copy from user space to kernel failed\n");
		return -EFAULT;
	}
	return 0;
}

/* Char driver unlocked_ioctl function : Called when ioctl() is called on the device
 * It is used to configure the device parameters
 */
long led74hc595_ioctl(struct file *f, unsigned int cmd,
		unsigned long arg)
{
	PINFO("In char driver ioctl() function\n");

	switch (cmd) {

	case FGETIO:
		break;

	/* If user application has admin access then only this command will execute */
	case FSETIO:

		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

/* Define the file operations structure of the char driver */
struct file_operations led74hc595_fops= {
	.owner 			= THIS_MODULE,
	.open			= led74hc595_open,
	.release		= led74hc595_release,
	.read			= led74hc595_read,
	.write			= led74hc595_write,
	.unlocked_ioctl = led74hc595_ioctl
};
static enum hrtimer_restart timer_timeout(struct hrtimer *timer)
{

	PINFO("Inside the timer function, with data\n");
	hrtimer_forward_now(timer, kt_period);
	return HRTIMER_RESTART;
	/* To make the timer periodic, uncomment the following line */
}

static void timer_init(void)
{
// Init timer
	kt_period = ktime_set(3, 10000);
	hrtimer_init(&htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
	htimer.function = timer_timeout;
	hrtimer_start(&htimer, kt_period, HRTIMER_MODE_REL);
}

static void timer_cancel(void)
{
	hrtimer_cancel(&htimer);
}

static int __init led74hc595_init(void)
{
	int i;
	int res;

	PINFO("In init() function\n");

	/* Get the device number dynamically */
	res = alloc_chrdev_region(&deviceno , minor, NMINORS , DRIVER_NAME);
	if(res <0) {

		PERR("register device no failed\n");
		return -1;
	}
	major = MAJOR(deviceno);

	/* create a class file with the name DRIVER_NAME such that it
	 *  will appear in /sys/class/<DRIVER_NAME
	 */
	led74hc595_class = class_create(THIS_MODULE , DRIVER_NAME);
	if(led74hc595_class == NULL) {
		PERR("class creation failed\n");
		return -1;
	}

	/* Create nMinors Device nodes , such users can access through nodes Ex :/dev/sample_cdev0 */
	for(i = 0; i < NMINORS; i++) {

		deviceno = MKDEV(major, minor + i);

		/* Attach file_operations to cdev and add the device to the linux kernel */
		cdev_init(&devices[i].led , &led74hc595_fops);
		cdev_add(&devices[i].led, deviceno,1);

		/* Create the Device node in /dev/ directory */
		led74hc595_device = device_create(led74hc595_class ,
				NULL , deviceno , NULL ,"ledhc%d",i);
		if(led74hc595_device == NULL) {

			class_destroy(led74hc595_class);
			PERR("device creation failed\n");
			return -1;
		}

		devices[i].nMinor = minor + i;
	}
	timer_init();

	PINFO("Out init() function\n");
	return 0;
}

/* Cleanup function */
static void __exit led74hc595_exit(void)
{
	int i;

	PINFO("In exit() function\n");

	/* Remove cdev and device nodes with linux kernel */
	timer_cancel();
	for(i = 0; i < NMINORS; i++) {

		deviceno = MKDEV(major, minor+i);
		cdev_del(&devices[i].led);
		device_destroy(led74hc595_class , deviceno);
	}

	/* Destroy the class we have created */
	class_destroy(led74hc595_class);

	/* Unregister the device number from linux kernel */
	deviceno=MKDEV(major, minor);
	unregister_chrdev_region(deviceno, NMINORS);
	PINFO("Out exit() function\n");
}

module_init(led74hc595_init)
module_exit(led74hc595_exit)
