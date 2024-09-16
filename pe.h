#ifndef PE_H
#define PE_H
#include <QString>
#include <QList>
#include <QPoint>

class PE
{
public:
    PE(
        const QString   &id,
        const QString   &type,
        const float     &lat = 0,
        const float     &lon = 0,
        const float     &altitude = 10000,
        const float     &speed = 100,
        const QString   &apd = "MED",
        const QString   &priority = "MED",
        const bool      &jam = false,
        const bool      &ghost = false) :
        /* Identification */id(id), type(type),
        /* Location       */lat(lat), lon(lon),speed(speed),heading(0), altitude(altitude), climbRate(0), ghost(ghost),
        /* EWBM Priority  */apd(apd), priority(priority),
        /* EWBM Settings  */jam(jam),
        /* EWAM Settings  */state("start"), currentRoutePoint(0)
    {
        category = getCategory(type);
        listOfRelThreats.clear();
        route.clear();
    }

    enum PECategory {
        Fighter,
        Weapon,
        Attack,
        Bomber,
        C2,
        EW,
        Tanker,
        UAV,
        Rotary,
        Transport,
        Other
    };

    PECategory getCategory(QString name) const
    {
        //Initialise PE Categories Map
        //FIXME: This is a quick and dirty naive solution - we should use a config file that creates a QMap<> for any final versions,  however for
        //the test TRL version we can adjust this easily to add and remove potential items.
        static QStringList fighters = {"F18", "F35", "F15", "F16", "F22"};
        static QStringList c2 =       {"E7", "E2", "P3", "P8", "E3", "E4B"};
        static QStringList attack =   {"A10", "AC130J"};
        static QStringList transport = {"C5", "C17", "C20", "C21", "C32", "C37", "C40", "C130", "C27", "KA350", "DF7X", };
        static QStringList bomber =    {"B1B", "B2", "B21", "B52"};
        static QStringList rotary =    {"CV22", "HH60", "MH60", "UH1N", "AH6", "AH64", "UH60", "UH72"};
        static QStringList tanker =    {"KC10", "KC30", "KC46", "KC135"};
        static QStringList ew =        {"EA18G", "EC130H"};
        static QStringList uav =       {"MQ1", "MQ9", "RQ4", "MQ28", "XQ58", "XQ67"};
        static QStringList weapon =    {"AGM65", "AGM88", "AGM84", "AGM114"};

        if      (fighters.contains(name))   return PECategory::Fighter;
        else if (c2.contains(name))         return PECategory::C2;
        else if (attack.contains(name))     return PECategory::Attack;
        else if (transport.contains(name))  return PECategory::Transport;
        else if (bomber.contains(name))     return PECategory::Bomber;
        else if (rotary.contains(name))     return PECategory::Rotary;
        else if (tanker.contains(name))     return PECategory::Tanker;
        else if (ew.contains(name))         return PECategory::EW;
        else if (uav.contains(name))        return PECategory::UAV;
        else if (weapon.contains(name))     return PECategory::Weapon;
        else                                return PECategory::Other;
    }



    //Identification
    QString     uci_uuid;             //The UUID for the platform
    QString     id;                   //Callsign
    QString     type;                 //Type of aircraft expressed as a string (eg. EA18G)
    PECategory  category;             //Category of aircraft expressed as a string (eg. fighter, bomber, etc)

    //Location
    float       lat;                  //Currently Cartesian y position - TODO: correct for better maps
    float       lon;                  //Currently Cartesian x position - TODO: correct for better maps
    float       speed;                //TODO: Units/sec
    float       heading;              //Radians
    float       altitude;             //TODO: Units
    float       climbRate;            //TODO: Units/sec
    bool        ghost;                //true if this is a ghost track

    //EWBM Settings for the PE
    QString     apd;                  //XLO/LO/MED/HI/XHI
    QString     priority;             //XLO/LO/MED/HI/XHI

    //FIXME: All of the items below are currently unutilised in the EWAM version of the code
    bool        jam;                  //true if this aircraft is protected by ownship jamming
    QList<QString> listOfRelThreats;  //list of UUIDs for relevant threats

    //Simulated EWAM only information
    QString     state;                //Flight state for the EWAM - start/transit/crashing/crashed
    int         currentRoutePoint;    //Current point in the route
    QList<QPointF> route;             //Route - used by EWAM simulator
};

#endif // PE_H
