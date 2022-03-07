#ifndef __AACURVE_H__
#define __AACURVE_H__

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>

namespace aaAaa{
	enum SplineType {
		SPLINE_CUBIC = 0,
		SPLINE_QUADRATIC,
		SPLINE_LINEAR,
		SPLINE_COUNT,
	};

    struct aaPoint{
        double t, y;

        aaPoint(void) : t(0), y(0){}
        aaPoint(double at, double ay) : t(at), y(ay){}
    };
    typedef aaPoint Vector2;

    struct Vector3{
        double x, y, z;
        Vector3(void): x(0), y(0), z(0){}
        Vector3(double ax, double ay, double az)
            : x(ax), y(ay), z(az){

        }
    };

    // spline data
    struct aaSpline{
        
        static std::string spline_name[SPLINE_COUNT];

        int type;

        typedef std::vector<aaPoint> KnotsList;
        std::string name;
        //   std::map<double, aaPoint> knots;
        KnotsList knots;

        bool bLimited;
        double limit_left, limit_top, limit_right, limit_bottom;

        aaSpline(void)
            : type(SPLINE_QUADRATIC), bLimited(false), limit_left(0.0f),
            limit_top(0.0f), limit_right(0.0f), limit_bottom(0.0f){

        }

        aaSpline(const std::string &spline_name)
            : type(SPLINE_CUBIC), name(spline_name), bLimited(false), limit_left(0.0f),
            limit_top(0.0f), limit_right(0.0f), limit_bottom(0.0f){

        }

        void setKnots(const std::vector<std::pair<double, double> > &nots){
            knots.clear();
            for(int i=0; i<nots.size(); ++i){
                //knots.insert(std::make_pair(nots[i].first, aaPoint(nots[i].first, nots[i].second)));
                knots.push_back(aaPoint(nots[i].first, nots[i].second));
            }
        }

        void addKnots(std::pair<double, double> knot){
            //knots.insert(std::make_pair(not.first, aaPoint(not.first, not.second)));
            KnotsList::iterator it = knots.begin();
            for(; it!=knots.end(); ++it){
                if(knot.first < (*it).t)
                    break;
                else if(knot.first == (*it).t){
                    return ;
                }
            }

            knots.insert(it, aaPoint(knot.first, knot.second));
        }

        int addKnots(aaPoint knot){
            //knots.insert(std::make_pair(not.t, not));
            int count = 0;
            KnotsList::iterator it = knots.begin();
            for(; it!=knots.end(); ++it){
                if(knot.t < (*it).t)
                    break;
                else if(knot.t == (*it).t){
                    return -1;
                }
                ++count;
            }

            knots.insert(it, knot);
            return count;
        }

        void setLimit(double left, double top, double right, double bottom){
            bLimited = true;
            limit_left = left;
            limit_top = top;
            limit_right = right;
            limit_bottom = bottom;
        }

        void removeLimit(void){
            bLimited = false;
        }

        void setName(const std::string &name){
            this->name = name;
        }

        size_t size(void) const{ return knots.size(); }
    };

    // spline function

    class aaCurve{
    protected:
        std::string m_name;
        int m_knum;
        
        double *m_t;
        double *m_knot;

    public:
 //       aaCurve(const std::string &name, int knum, double *t, double *knot);
		aaCurve(const aaSpline& spline_data);
		aaCurve(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data);
        virtual ~aaCurve(void);

        std::string name(void) const{ return m_name; }

        virtual bool getValue(double t, double &value) = 0;

    };

    typedef std::auto_ptr<aaCurve> aaCurvePtr;

    class aaCubicSpline : public aaCurve{
    private:
        double *m_ddp;
    
    public:
        aaCubicSpline(const aaSpline &spline_data);
        aaCubicSpline(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data);
		virtual ~aaCubicSpline(void);

        virtual bool getValue(double t, double &value);
    };
   
    class aaQuadraticSpline : public aaCurve{

    public:
        aaQuadraticSpline(const aaSpline &spline_data);
        aaQuadraticSpline(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data);
		virtual ~aaQuadraticSpline(void);

        virtual bool getValue(double t, double &value);
    };

    class aaLinearSpline : public aaCurve{

    public:
        aaLinearSpline(const aaSpline &spline_data);
        aaLinearSpline(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data);
		virtual ~aaLinearSpline(void);

        virtual bool getValue(double t, double &value);
    };
    //////////////////////////////////////////////////////////////////////////

    class aaCurveFactory{
    private:
        aaCurveFactory(void);
        aaCurveFactory(const aaCurveFactory&);
    public:
        static aaCurvePtr createCurve(const aaSpline &spline_data){
            switch(spline_data.type){
                case aaAaa::SplineType::SPLINE_CUBIC:
                    return aaCurvePtr(new aaCubicSpline(spline_data));
                case aaAaa::SplineType::SPLINE_QUADRATIC:
                    return aaCurvePtr(new aaQuadraticSpline(spline_data));
                case aaAaa::SplineType::SPLINE_LINEAR:
                    return aaCurvePtr(new aaLinearSpline(spline_data));
                default:
                    return aaCurvePtr(0);
            }
        }
		static aaCurvePtr createCurve(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data, SplineType type) {
			switch (type) {
            case aaAaa::SplineType::SPLINE_CUBIC:
				return aaCurvePtr(new aaCubicSpline(spline_data));
			case aaAaa::SplineType::SPLINE_QUADRATIC:
				return aaCurvePtr(new aaQuadraticSpline(spline_data));
			case aaAaa::SplineType::SPLINE_LINEAR:
				return aaCurvePtr(new aaLinearSpline(spline_data));
			default:
				return aaCurvePtr(0);
			}
		}
    };

}

#endif // __AASPLINE_H__

