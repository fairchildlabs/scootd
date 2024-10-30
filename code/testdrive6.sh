

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

screen -d -m ./scootd

./scootdtest 0x10505
echo "Sleeping 60"
sleep 60

./scootdtest 0x10101
echo "Sleeping 60"
sleep 60

./scootdtest 0x110505
echo "Sleeping 60"
sleep 60

./scootdtest 0x110101
echo "Sleeping 60"
sleep 60

./scootdtest 0x11515
echo "Sleeping 60"
sleep 60

./scootdtest 0x11111
echo "Sleeping 60"
sleep 60

./scootdtest 0x12525
echo "Sleeping 60"
sleep 60

./scootdtest 0x12121
echo "Sleeping 60"
sleep 60

./scootdtest 0
echo "End Test Drive"
sleep 60


