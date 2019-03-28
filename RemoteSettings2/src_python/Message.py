
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


def ParseMessage(message):
    #split the string at the first blank.
    dst,_,rest=message.partition(' ')
    #then at the second blank
    cmd,_,data=rest.partition(' ')
    return dst,cmd,data


def ParseMessageData(data):
    key,value=data.split("=")
    return key,value


def BuildMessageCHANGE(id,key,value):
    return id+" "+"CHANGE "+key+"="+value


#me is either ground or air (G or A)
def BuildMessageCHANGE_OK(id,key,value):
    return (id+" "+"CHANGE_OK"+" "+key+"="+value)


def BuildMessageGET(id,key):
    return id+" "+"GET "+key


def BuildMessageGET_OK(id,key,value):
    return (id+" "+"GET_OK"+" "+key+"="+value)



def BuildMessageERROR():
    return "ERROR"


def BuildMessageHELLO(id):
    return id+" "+"HELLO"

def BuildMessageHELLO_OK(id):
    return id+" "+"HELLO_OK"