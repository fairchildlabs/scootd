#./scootd &


#	struct
#	{
#		unsigned int video        :  1;
#		unsigned int frame_rate   :  1;
#		unsigned int raw          :  1;
#		unsigned int rsvd         :  1;
#		unsigned int resolution   :  4;
#
#	} vid[2];
#


./scootdtest 0x0005
echo "Sleeping 60"
sleep 60

./scootdtest 0x0001
echo "Sleeping 60"
sleep 60


./scootdtest 0x0015
echo "Sleeping 60"
sleep 60

./scootdtest 0x0011
echo "Sleeping 60"
sleep 60

./scootdtest 0x0025
echo "Sleeping 60"
sleep 60

./scootdtest 0x0021
echo "Sleeping 60"
sleep 60


./scootdtest 0x0035
echo "Sleeping 60"
sleep 60

./scootdtest 0x0031
echo "Sleeping 60"
sleep 60

./scootdtest 0x0045
echo "Sleeping 60"
sleep 60

./scootdtest 0x0041
echo "Sleeping 60"
sleep 60

./scootdtest 0
echo "DONE TEST DRIVE"

