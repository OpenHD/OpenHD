
#Messages always have the format
#<ID (Destination or source) <Command> <Optional Data> \n
#where ID is either G (Ground pi), A (Air pi) or GA (Ground and air pi)


#All supported commands:
# 'GET' optain a key-value-pair
# 'CHANGE' change a key-value-pair
# 'HELLO' ping the client


#All supported responses:
# 'GET_OK' return a key-value-pair
# 'CHANGE_OK' changed a key-value-pair

SEPERATOR="?"

def ParseMessage(message):
    #split the string at the first blank.
    dst,_,rest=message.partition(SEPERATOR)
    #then at the second blank
    cmd,_,data=rest.partition(SEPERATOR)
    return dst,cmd,data


#return list of data chuncks
def SeperateMessageData(data):
    return data.split(SEPERATOR)


#return list of key,value pairs
def SeperateMessageDataAndSplit(data):
    seperatedData=SeperateMessageData(data)
    ret=[]
    for x in seperatedData:
        key,value=x.split("=")
        ret.append((key,value))
    return ret


#me is either ground or air (G or A)
def BuildMessageCHANGE_OK(id,keyValuePairs):
    ret=id+SEPERATOR+"CHANGE_OK"+SEPERATOR
    for (key,value) in keyValuePairs:
        ret+=(key+"="+value+SEPERATOR)
    return ret[0:len(ret)-1]


def BuildMessageGET_OK(id,keyValuePairs):
    ret=id+SEPERATOR+"GET_OK"+SEPERATOR
    for (key,value) in keyValuePairs:
        ret+=(key+"="+value+SEPERATOR)
    return ret[0:len(ret)-1]


def BuildMessageERROR():
    return "ERROR"


def BuildMessageHELLO(id):
    return id+SEPERATOR+"HELLO"

def BuildMessageHELLO_OK(id):
    return id+SEPERATOR+"HELLO_OK"

