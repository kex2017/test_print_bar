APPLICATION = lora-p2p-sctm

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# If no BOARD is found in the environment, use this default:
BOARD ?= kl_p2p
# BOARD ?= nucleo-f103rb
# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# CFLAGS += -Wno-cast-function-type
# CFLAGS += -Wno-pointer-sign
# CFLAGS += -Wno-incompatible-pointer-types

ROM_START_ADDR ?=0x08000000
# Change this to 0 show compiler invocation lines by default:
QUIET ?= 0

FEATURES_REQUIRED += periph_timer
FEATURES_REQUIRED += periph_uart
FEATURES_REQUIRED += periph_uart_modecfg
FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_spi
# FEATURES_REQUIRED += periph_rtt

USEMODULE += od
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer
USEMODULE += isrpipe
USEMODULE += isrpipe_read_timeout

DRIVER ?= sx1276

USEMODULE += $(DRIVER)

include $(RIOTBASE)/Makefile.include