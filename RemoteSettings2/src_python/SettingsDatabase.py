from FileParser import read_bash_file,read_header_file,replace_in_bash_file,replace_in_header_file

#the goal of the SettingsDatabase.py fie is to create change and get functions that only take key-value pairs
#and determine the right location (which settings file contains the key-value tuple) automatically


OSDSettingsFile = None
OpenHDSettingsFile = None
JoyconfigSettingsFile = None


def createSettingsDatabase(me):
    global OSDSettingsFile
    global OpenHDSettingsFile
    global JoyconfigSettingsFile
    #if me is either G or A we are on a test build not on the OpenHD system, but on my local machine
    #that has 2 folders to emulate ground and air pi
    if(me=='G'):
        OSDSettingsFile = "test_ground/osdconfig.txt"
        OpenHDSettingsFile = "test_ground/openhd-settings-1.txt"
        JoyconfigSettingsFile = "test_ground/joyconfig.txt"
    elif(me=='A'):
        OSDSettingsFile = "test_air/osdconfig.txt"
        OpenHDSettingsFile = "test_air/openhd-settings-1.txt"
        JoyconfigSettingsFile = "test_air/joyconfig.txt"
    #on the openHD system everything resides in the /boot directory, no matter if we are on the air or ground pi
    else:
        OSDSettingsFile = "/boot/osdconfig.txt"
        OpenHDSettingsFile = "/boot/openhd-settings-1.txt"
        JoyconfigSettingsFile = "/boot/joyconfig.txt"

    listOfAllSettingsInDictionaries=[]
    listOfAllSettingsInDictionaries.append((OpenHDSettingsFile,read_bash_file(OpenHDSettingsFile)))
    listOfAllSettingsInDictionaries.append((OSDSettingsFile,read_header_file(OSDSettingsFile)))
    listOfAllSettingsInDictionaries.append((JoyconfigSettingsFile,read_header_file(JoyconfigSettingsFile)))
    return listOfAllSettingsInDictionaries


def getValueForKey(database,key):
    #handle the version as an extra key
    if(key=="VERSION"):
        return "OpenHD_1.1.1"
    for  item in database:
        dictionary=item[1]
        value=dictionary.get(key)
        if(value):
            return value
    return None


#determine in which file this key is stored
#database: a list of dictionaries
#This means: the list contains tuples in the form of (filename : dictionary)
#and dictionary contains key-value tuples
def determineSettingsLocation(database,key):
    for  item in database:
        filename=item[0]
        dictionary=item[1]
        value=dictionary.get(key)
        if(value):
            return filename
    return None


#find the file that contains key
#change the file the new key-value pair
#dont't forget to update the database ofter modifying some settings
def changeSetting(database,key,value):
    place=determineSettingsLocation(database,key)
    print('changing',key,value,'in ',place)
    if(place==OpenHDSettingsFile):
        replace_in_bash_file(OpenHDSettingsFile,key,value)
    elif (place==OSDSettingsFile):
        replace_in_header_file(OSDSettingsFile,key,value)
    elif(place==JoyconfigSettingsFile):
        replace_in_header_file(JoyconfigSettingsFile,key,value)
    else:
        print("not found",place,OpenHDSettingsFile)
        return False
    return True



def test1():
    print("read files into dictionary and print dictionary's values")
    dictionary=read_header_file(OSDSettingsFile)
    for i in dictionary:
        print(i,dictionary[i])
    print('------------------------')
    dictionary=read_bash_file(OpenHDSettingsFile)
    for i in dictionary:
        print(i,dictionary[i])

def test2():
    allSettingsInDictionary={}
    #allSettingsInDictionary.update(read_bash_file(OpenHDSettingsFile))
    #allSettingsInDictionary.update(read_header_file(OSDSettingsFile))
    allSettingsInDictionary.update(read_header_file(JoyconfigSettingsFile))
    for i in allSettingsInDictionary:
        print(i,allSettingsInDictionary[i])

#replace 1 value in a header and 1 value in a bash file
def test3():
    replace_in_bash_file(OpenHDSettingsFile,"FREQ","2372")
    replace_in_header_file(OSDSettingsFile,"DOWNLINK_RSSI_POS_X", "13")

def test4():
    listOfAllSettingsInDictionaries=createSettingsDatabase('G')
    print("contained in:",determineSettingsLocation(listOfAllSettingsInDictionaries,"FREQ"))


def test5():
    listOfAllSettingsInDictionaries=createSettingsDatabase('A')
    changeSetting(listOfAllSettingsInDictionaries,"FC_RC_BAUDRATE","1200")

#test1()
#test2()
#test3()
#test4()
#test5()