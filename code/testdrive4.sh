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


./scootdtest 0x0505
echo "Sleeping 60"
sleep 60

./scootdtest 0x0101
echo "Sleeping 60"
sleep 60


./scootdtest 0x1515
echo "Sleeping 60"
sleep 60

./scootdtest 0x1111
echo "Sleeping 60"
sleep 60

./scootdtest 0x2525
echo "Sleeping 60"
sleep 60

./scootdtest 0x2121
echo "Sleeping 60"
sleep 60


./scootdtest 0x3535
echo "Sleeping 60"
sleep 60

./scootdtest 0x3131
echo "Sleeping 60"
sleep 60

./scootdtest 0x4545
echo "Sleeping 60"
sleep 60

./scootdtest 0x4141
echo "Sleeping 60"
sleep 60

./scootdtest 0
echo "DONE TEST DRIVE"

