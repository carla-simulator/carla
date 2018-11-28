"""
Actor registry class for carla-id mapping
"""


class ActorIdRegistry(object):

    """
    Registry class to map carla-ids (potentially 64 bit) to increasing numbers (usually not exceeding 32 bit)
    """

    def __init__(self):
        """
        Constructor
        """
        self.id_lookup_table = {}

    def get_id(self, actor_id):
        """
        Return a unique counting id for the given actor_id

        :param actor_id: the id of a carla.Actor object
        :type actor_id: int64
        :return: mapped id of the actor (unique increasing counter value)
        :rtype: uint32
        """
        if actor_id not in self.id_lookup_table:
            self.id_lookup_table[actor_id] = len(self.id_lookup_table) + 1
        return self.id_lookup_table[actor_id]

    def get_id_string(self, actor_id):
        """
        Return a string of a unique counting id for the given actor_id

        :param actor_id: the id of a carla.Actor object
        :type actor_id: int64
        :return: string with leading zeros of mapped id of the actor
            (unique increasing counter value)
        :rtype: string
        """
        mapped_id = self.get_id(actor_id)
        return "{:03d}".format(mapped_id)
