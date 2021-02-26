all:
	gcc -o vesh vesh.c 
	
run:
	./vesh
	
clean:
	rm ./vesh
