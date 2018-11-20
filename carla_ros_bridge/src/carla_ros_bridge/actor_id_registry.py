    
class ActorIdRegistry:
    
    def __init__(self): 
        self.id_lookup_table = {}
    
    
    def get_id(self, actor_id):
        """
        Return a unique counting id for the given actor_id

        :param actor_id:
        :return:
        """
        if actor_id not in self.id_lookup_table:
            self.id_lookup_table[actor_id] = len(self.id_lookup_table) + 1
        return self.id_lookup_table[actor_id]

    def get_id_string(self, actor_id):
        """
        Return a unique counting id for the given actor_id as string with at least three digits

        :param actor_id:
        :return:
        """
        id = self.get_id(actor_id)
        return "{:03d}".format(id)
