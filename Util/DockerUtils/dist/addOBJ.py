import sys
import re

# check parameters
if (len(sys.argv) < 3):
	print("Usage: result.obj to_add.obj [material_id]")
	sys.exit(1)

#----------------------------------
# read total of vertex already used
#----------------------------------
totalVertex = 0
try:
	f = open(sys.argv[1], "rt")
	lines = f.readlines()
	for line in lines:
		if (line[:2] == "v "):
			totalVertex += 1
except:
	f = open(sys.argv[1], "wt")
f.close()

#--------------------------------------
# read the min face index of the source
#--------------------------------------
minIndex = 10000000
try:
	f = open(sys.argv[2], "rt")
	lines = f.readlines()
	for line in lines:
		if (line[:2] == "f "):
			# get the indexes (index[/uv/normal] index[/uv/normal] index[/uv/normal])
			res = re.match(r"f (\d+).*? (\d+).*? (\d+)", line)
			if (not res):
				print("Problem parsing face indexes '%s'" % line)
				sys.exit(1)
			else:
				# get the min index
				if (minIndex > int(res.groups(0)[0])):
					minIndex = int(res.groups(0)[0])
				if (minIndex > int(res.groups(0)[1])):
					minIndex = int(res.groups(0)[1])
				if (minIndex > int(res.groups(0)[2])):
					minIndex = int(res.groups(0)[2])
except:
	minIndex = 0
f.close()

#----------------
# read obj to add
#----------------
f = open(sys.argv[2], "rt")
if (not f):
	print("Could not open %s file" % sys.argv2[2])
	sys.exit(1)
lines = f.readlines()
f.close()
d = open(sys.argv[1], "at")
for line in lines:
	# object
	if (line[:2] == "o "):
		d.write(line)
	# group
	elif (line[:2] == "g "):
		d.write(line)
	# vertex
	elif (line[:2] == "v "):
		d.write(line)
	# material
	elif (line[:2] == "us"):
		if (len(sys.argv) > 3):
			d.write("usemtl %s\n" % sys.argv[3])
		else:
			d.write(line)
	# face
	elif (line[:2] == "f "):
		# get the indexes (index[/uv/normal] index[/uv/normal] index[/uv/normal])
		res = re.match(r"f (\d+).*? (\d+).*? (\d+)", line)
		if (not res):
			print("Problem parsing face indexes '%s'" % line)
			sys.exit(1)
		d.write("f %d %d %d\n" % (totalVertex + int(res.groups(0)[0]) - minIndex + 1,
			 	   				  totalVertex + int(res.groups(0)[1]) - minIndex + 1,
			 	   				  totalVertex + int(res.groups(0)[2]) - minIndex + 1))
d.close()
