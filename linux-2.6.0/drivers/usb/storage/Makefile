#
# Makefile for the USB Mass Storage device drivers.
#
# 15 Aug 2000, Christoph Hellwig <hch@infradead.org>
# Rewritten to use lists instead of if-statements.
#

EXTRA_CFLAGS	:= -Idrivers/scsi

obj-$(CONFIG_USB_STORAGE)	+= usb-storage.o

usb-storage-obj-$(CONFIG_USB_STORAGE_DEBUG)	+= debug.o
usb-storage-obj-$(CONFIG_USB_STORAGE_HP8200e)	+= shuttle_usbat.o raw_bulk.o
usb-storage-obj-$(CONFIG_USB_STORAGE_SDDR09)	+= sddr09.o raw_bulk.o
usb-storage-obj-$(CONFIG_USB_STORAGE_SDDR55)	+= sddr55.o raw_bulk.o
usb-storage-obj-$(CONFIG_USB_STORAGE_FREECOM)	+= freecom.o
usb-storage-obj-$(CONFIG_USB_STORAGE_DPCM)	+= dpcm.o
usb-storage-obj-$(CONFIG_USB_STORAGE_ISD200)	+= isd200.o
usb-storage-obj-$(CONFIG_USB_STORAGE_DATAFAB)	+= datafab.o raw_bulk.o
usb-storage-obj-$(CONFIG_USB_STORAGE_JUMPSHOT)	+= jumpshot.o raw_bulk.o

usb-storage-objs :=	scsiglue.o protocol.o transport.o usb.o \
			initializers.o $(usb-storage-obj-y)
