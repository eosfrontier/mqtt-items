# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

env := lights

all:
	pio -f -c vim run -e ${env}

upload:
	pio -f -c vim run --target upload -e ${env}

clean:
	pio -f -c vim run --target clean -e ${env}

program:
	pio -f -c vim run --target program -e ${env}

uploadfs:
	pio -f -c vim run --target uploadfs -e ${env}

update:
	pio -f -c vim update
