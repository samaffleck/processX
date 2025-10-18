#include <string>
#include <vector>

namespace unitop {

    class Valve {
        private:
            std::string name;
            std::string fluid;
            double mass_flow_rate;
            double pressure_drop;
        public:
            Valve(const std::string& name, const std::string& fluid, const double& mass_flow_rate, const double& pressure_drop);
            ~Valve();
            void open();
            void close();
            void set_mass_flow_rate(const double& mass_flow_rate);
            void set_pressure_drop(const double& pressure_drop);
    };

    Valve::Valve(const std::string& name, const std::string& fluid, const double& mass_flow_rate, const double& pressure_drop)
        : name(name), fluid(fluid), mass_flow_rate(mass_flow_rate), pressure_drop(pressure_drop) {
    }

    Valve::~Valve() {
    }
    
    void Valve::open() {
    }

    void Valve::close() {
    }

    void Valve::set_mass_flow_rate(const double& mass_flow_rate) {
    }
    
    void Valve::set_pressure_drop(const double& pressure_drop) {
    }

} // namespace unitop
