
class DebugDrawing:
    class DebugLine:
        ...

    class DebugPoint:
        ...

    class DebugPolygon:
        ...

    class NullDeleter:
        ...

    def drawLine(self, arg1: DebugDrawing, line: object, color: str, ns: str) -> None:
        """

        drawLine( (DebugDrawing)arg1, (object)line [, (str)color='white' [, (str)ns='']]) -> None :

            C++ signature :
                void drawLine(ad::rss::unstructured::DebugDrawing {lvalue},boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, std::vector, std::allocator> [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='white' [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='']])
        """
        ...

    def drawPolygon(self, arg1: DebugDrawing, polygon: object, color: str, ns: str) -> None:
        """

        drawPolygon( (DebugDrawing)arg1, (object)polygon [, (str)color='white' [, (str)ns='']]) -> None :

            C++ signature :
                void drawPolygon(ad::rss::unstructured::DebugDrawing {lvalue},boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian>, false, true, std::vector, std::vector, std::allocator, std::allocator> [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='white' [,std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >='']])
        """
        ...

    def enable(self, arg1: DebugDrawing, value: bool) -> None:
        """

        enable( (DebugDrawing)arg1, (bool)value) -> None :

            C++ signature :
                void enable(ad::rss::unstructured::DebugDrawing {lvalue},bool)
        """
        ...

    def getInstance(self) -> DebugDrawing:
        """

        getInstance() -> DebugDrawing :

            C++ signature :
                std::shared_ptr<ad::rss::unstructured::DebugDrawing> getInstance()
        """
        ...

    def isEnabled(self, arg1: DebugDrawing) -> bool:
        """

        isEnabled( (DebugDrawing)arg1) -> bool :

            C++ signature :
                bool isEnabled(ad::rss::unstructured::DebugDrawing {lvalue})
        """
        ...

    @property
    def mEnabled(self) -> bool: ...

    @property
    def mLines(self) -> vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_: ...

    @property
    def mPolygons(self) -> vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_: ...

    def reset(self, arg1: DebugDrawing) -> None:
        """

        reset( (DebugDrawing)arg1) -> None :

            C++ signature :
                void reset(ad::rss::unstructured::DebugDrawing {lvalue})
        """
        ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_:
    def append(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_, arg2: DebugDrawing.DebugLine) -> None:
        """

        append( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_)arg1, (DebugLine)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::unstructured::DebugDrawing::DebugLine, std::allocator<ad::rss::unstructured::DebugDrawing::DebugLine> > {lvalue},ad::rss::unstructured::DebugDrawing::DebugLine)
        """
        ...

    def extend(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_, arg2: object) -> None:
        """

        extend( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::unstructured::DebugDrawing::DebugLine, std::allocator<ad::rss::unstructured::DebugDrawing::DebugLine> > {lvalue},boost::python::api::object)
        """
        ...

    def insert(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_, arg2: int, arg3: DebugDrawing.DebugLine) -> None:
        """

        insert( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_)arg1, (int)arg2, (DebugLine)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::unstructured::DebugDrawing::DebugLine, std::allocator<ad::rss::unstructured::DebugDrawing::DebugLine> > {lvalue},long,ad::rss::unstructured::DebugDrawing::DebugLine)
        """
        ...

    def reverse(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_) -> None:
        """

        reverse( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugLine_greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::unstructured::DebugDrawing::DebugLine, std::allocator<ad::rss::unstructured::DebugDrawing::DebugLine> > {lvalue})
        """
        ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_:
    def append(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_, arg2: DebugDrawing.DebugPoint) -> None:
        """

        append( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_)arg1, (DebugPoint)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::unstructured::DebugDrawing::DebugPoint, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPoint> > {lvalue},ad::rss::unstructured::DebugDrawing::DebugPoint)
        """
        ...

    def extend(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_, arg2: object) -> None:
        """

        extend( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::unstructured::DebugDrawing::DebugPoint, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPoint> > {lvalue},boost::python::api::object)
        """
        ...

    def insert(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_, arg2: int, arg3: DebugDrawing.DebugPoint) -> None:
        """

        insert( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_)arg1, (int)arg2, (DebugPoint)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::unstructured::DebugDrawing::DebugPoint, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPoint> > {lvalue},long,ad::rss::unstructured::DebugDrawing::DebugPoint)
        """
        ...

    def reverse(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_) -> None:
        """

        reverse( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPoint_greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::unstructured::DebugDrawing::DebugPoint, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPoint> > {lvalue})
        """
        ...

class vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_:
    def append(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_, arg2: DebugDrawing.DebugPolygon) -> None:
        """

        append( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_)arg1, (DebugPolygon)arg2) -> None :

            C++ signature :
                void append(std::vector<ad::rss::unstructured::DebugDrawing::DebugPolygon, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPolygon> > {lvalue},ad::rss::unstructured::DebugDrawing::DebugPolygon)
        """
        ...

    def extend(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_, arg2: object) -> None:
        """

        extend( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_)arg1, (object)arg2) -> None :

            C++ signature :
                void extend(std::vector<ad::rss::unstructured::DebugDrawing::DebugPolygon, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPolygon> > {lvalue},boost::python::api::object)
        """
        ...

    def insert(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_, arg2: int, arg3: DebugDrawing.DebugPolygon) -> None:
        """

        insert( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_)arg1, (int)arg2, (DebugPolygon)arg3) -> None :

            C++ signature :
                void insert(std::vector<ad::rss::unstructured::DebugDrawing::DebugPolygon, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPolygon> > {lvalue},long,ad::rss::unstructured::DebugDrawing::DebugPolygon)
        """
        ...

    def reverse(self, arg1: vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_) -> None:
        """

        reverse( (vector_less_ad_scope_rss_scope_unstructured_scope_DebugDrawing_scope_DebugPolygon_greater_)arg1) -> None :

            C++ signature :
                void reverse(std::vector<ad::rss::unstructured::DebugDrawing::DebugPolygon, std::allocator<ad::rss::unstructured::DebugDrawing::DebugPolygon> > {lvalue})
        """
        ...
