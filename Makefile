all:
	(cd src; make; mv axisfix ..; make clean)

clean:
	rm -rf axisfix
