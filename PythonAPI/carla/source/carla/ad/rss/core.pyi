from typing import overload

from . import *

class RssCheck:

    @overload
    def calculateProperResponse(self, worldModel: world.WorldModel, situationSnapshot: situation.SituationSnapshot, rssStateSnapshot: state.RssStateSnapshot, properResponse: state.ProperResponse) -> bool:
        ...

    @overload
    def calculateProperResponse(self, worldModel: world.WorldModel, properResponse: state.ProperResponse) -> bool:
        """

        calculateProperResponse( (RssCheck)arg1, (WorldModel)worldModel, (ProperResponse)properResponse) -> bool :

            C++ signature :
                bool calculateProperResponse(ad::rss::core::RssCheck {lvalue},ad::rss::world::WorldModel,ad::rss::state::ProperResponse {lvalue})

        calculateProperResponse( (RssCheck)arg1, (WorldModel)worldModel, (SituationSnapshot)situationSnapshot, (RssStateSnapshot)rssStateSnapshot, (ProperResponse)properResponse) -> bool :

            C++ signature :
                bool calculateProperResponse(ad::rss::core::RssCheck {lvalue},ad::rss::world::WorldModel,ad::rss::situation::SituationSnapshot {lvalue},ad::rss::state::RssStateSnapshot {lvalue},ad::rss::state::ProperResponse {lvalue})
        """
        ...

class RssResponseResolving:
    def provideProperResponse(self, arg1: RssResponseResolving, currentStateSnapshot: state.RssStateSnapshot, response: state.ProperResponse) -> bool:
        """

        provideProperResponse( (RssResponseResolving)arg1, (RssStateSnapshot)currentStateSnapshot, (ProperResponse)response) -> bool :

            C++ signature :
                bool provideProperResponse(ad::rss::core::RssResponseResolving {lvalue},ad::rss::state::RssStateSnapshot,ad::rss::state::ProperResponse {lvalue})
        """
        ...

class RssSituationChecking:
    def checkSituations(self, arg1: RssSituationChecking, situationSnapshot: situation.SituationSnapshot, rssStateSnapshot: state.RssStateSnapshot) -> bool:
        """

        checkSituations( (RssSituationChecking)arg1, (SituationSnapshot)situationSnapshot, (RssStateSnapshot)rssStateSnapshot) -> bool :

            C++ signature :
                bool checkSituations(ad::rss::core::RssSituationChecking {lvalue},ad::rss::situation::SituationSnapshot,ad::rss::state::RssStateSnapshot {lvalue})
        """
        ...

class RssSituationExtraction:
    def extractSituations(self, arg1: RssSituationExtraction, worldModel: world.WorldModel, situationSnapshot: situation.SituationSnapshot) -> bool:
        """

        extractSituations( (RssSituationExtraction)arg1, (WorldModel)worldModel, (SituationSnapshot)situationSnapshot) -> bool :

            C++ signature :
                bool extractSituations(ad::rss::core::RssSituationExtraction {lvalue},ad::rss::world::WorldModel,ad::rss::situation::SituationSnapshot {lvalue})
        """
        ...
