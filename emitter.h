#ifndef EMITTER_H
#define EMITTER_H
#include <QString>
#include <QList>

class Emitter
{
public:
    Emitter(
        const QString   &id,
        const QString   &type,
        const QString   &category,
        const float     &lat = 0,
        const float     &lon = 0,
        const float     &freqMin = 8,
        const float     &freqMax = 12,
        const bool      &active = true,
        const QString   &eaPriority = "MED",
        const QString   &esPriority = "MED",
        const bool      &jamResponsible = true,
        const bool      &reactiveEligible = true,
        const bool      &preemptiveEligible = false,
        const bool      &consentRequired = false,
        const bool      &jam = false):
        /* Identification */id(id), type(type), category(category),
        /* Location       */lat(lat), lon(lon), altitude(0), heading(0), speed(0),
        /* Parameters     */freqMin(freqMin), freqMax(freqMax), active(active),
        /* EWBM Priority  */eaPriority(eaPriority), esPriority(esPriority),
        /* EWBM Settings  */jamResponsible(jamResponsible), reactiveEligible(reactiveEligible), preemptiveEligible(preemptiveEligible), consentRequired(consentRequired), operatorManaged(false),
        /* EWBM Jamming   */jam(jam), jamIneffective(0), jamEffective(0)
    {
        listOfRelPEs.clear();
    }

    //Identification
    QString         uci_uuid;         //The UUID for the emitter
    QString         id;               //ID string of the platform
    QString         type;             //What type of radar (eg. APG-79)
    QString         category;         //What category of radar is this (TA/MG/AI/EW)

    //Loction
    float           lat;              //Latitude - TODO: currently used as a cartesian Y position
    float           lon;              //Longitude - TODO: currently used as a cartesian X position
    float           altitude;         //Altitude in Units? TODO: units
    float           heading;          //Heading in radians
    float           speed;            //Speed in Units/second TODO: units

    //Parameters
    float           freqMin;          //Minimum frequency of the emitter (GHz)
    float           freqMax;          //Maximum frequency of the emitter (GHz)
    bool            active;           //true if the emitter is actively emitting

    //EWBM EA & ES priorities for this emitter
    QString         eaPriority;       //XLO/LO/MED/HI/XHI
    QString         esPriority;       //XLO/LO/MED/HI/XHI - currently unused

    //EWBM Settings for jamming this emitter
    bool            jamResponsible;
    bool            reactiveEligible;
    bool            preemptiveEligible;
    bool            consentRequired;
    bool            operatorManaged;  //true if the emitter is in Operator Priority

    //EWBM Jamming results
    bool            jam;              //true if we are jamming this emitter
    int             jamIneffective;   //Number of PEs ineffectively covered
    int             jamEffective;     //Number of PEs effectively covered

    //FIXME: All of the items below are currently unutilised in the EWAM version of the code
    QList<QString> listOfRelPEs;     //list of UUIDs for relevant Protected Entities


};

#endif // EMITTER_H
