import pickle

class BaseClass:
    instances = {}
    instancestanceID = 1
    
    @staticmethod
    def addFunc(function):
        if function.__name__ in __class__.__dict__:
            print("Warning, overwriting an existing Function!")
        setattr(__class__, function.__name__, function)
        
    @staticmethod
    def registerNewID(obj):
        if obj.id == -1:
            obj.id = __class__.instancestanceID
            __class__.instancestanceID += 1
            __class__.instances[obj.id] = obj
            
    @staticmethod
    def saveClass(file = None):
        if file is None:
            file = __class__.__name__+'Save.pkl'
        classDic = {}
        for obj in __class__.instances.values():
            classDic[obj.id] = obj.attributes
        with open(file, 'wb') as f:
            pickle.dump(classDic, f, protocol=None)
    
    @staticmethod
    def loadClass(file = None):
        if file is None:
            file = __class__.__name__+'Save.pkl'
        with open(file, 'wb') as f:
            objects = pickle.load(file)
        for ids, obj in objects.items():
            o = __class__(attributes = obj, registerNewID = False)
            __class__.instancestanceID = ids
            o.registerNewID(o)
    
    def __init__(self, register = True):
        self.id = -1
        if register:
            __class__.registerNewID(self)
    
    def save(self, file=None):
        '''Saves the Objects attributes'''
        if file is None:
            file = __class__.__name__+'ObjectSave.pkl'
        with open(file, 'wb') as f:
            pickle.dump(self.__dict__, f, protocol=None)
        
    def load(self, file=None):
        ''' loads the Object'''
        if file is None:
            file = __class__.__name__+'ObjectSave.pkl'
        with open(file, 'wb') as f:
            self.__dict__ = pickle.load(f)
            
def saveValues(values, file = 'values.pkl'):
    pickle.dump(values, file, protocol=None)
def loadValues(file = 'values.pkl'):
    return pickle.load(file)