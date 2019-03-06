import math


class GeometryUtils:

    def __init__(self):
        pass


    @staticmethod
    def distanceBetweenTwoPoints(pt1, pt2):
        (x1, y1) = pt1
        (x2, y2) = pt2
        return math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)