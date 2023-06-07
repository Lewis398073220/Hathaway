cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += -Iservices\bt_app \
                    -Iapps\key \
					-Iservices\bt_if_enhanced\inc \
					-Iutils\cqueue \
                    -Iapps\common \
					-Iservices\multimedia\audio\codec\sbc\inc \
					-Iservices\bt_app\a2dp_codecs\include \
					-Iplatform\hal \
					-Iapps\anc\inc \
					-Iapps\main \
					-Iapps\userapps



