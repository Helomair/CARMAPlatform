namespace example {
class LightsOn : public lanelet::RegulatoryElement {  // we have to inherit from the abstract regulatoryElement
 public:
  // lanelet2 looks for this string when matching the subtype of a regulatory element to the respective type
  static constexpr char RuleName[] = "lights_on";

  // returns the line where we are supposed to stop
  lanelet::ConstLineString3d fromWhere() const {
    return getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine).front();
  }

 private:
  LightsOn(lanelet::Id id, lanelet::LineString3d fromWhere)
      : RegulatoryElement{std::make_shared<lanelet::RegulatoryElementData>(id)} {
    parameters().insert({lanelet::RoleNameString::RefLine, {fromWhere}});
  }

  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class lanelet::RegisterRegulatoryElement<LightsOn>;
  explicit LightsOn(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data) {}
};
