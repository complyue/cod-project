// Generated from the finite selected semantic artifact.
struct Descriptor { TypeId identity; const char *name; const char *semantic_key; bool abstract; std::size_t size, align; };
constexpr TypeId T_Datum = 13893604854732947457ULL;
constexpr TypeId T_Plan = 13893604854732947458ULL;
constexpr TypeId T_LiteralPlan = 13893604854732947459ULL;
constexpr TypeId T_SourceLocated = 13893604854732947460ULL;
constexpr TypeId T_Scalar = 13893604854732947461ULL;
constexpr TypeId T_Quantity = 13893604854732947462ULL;
constexpr TypeId T_Destination = 13893604854732947463ULL;
constexpr TypeId T_AddressDestination = 13893604854732947464ULL;
constexpr TypeId T_Summary = 13893604854732947465ULL;
constexpr TypeId T_IntegerLiteral = 13893604854732947473ULL;
constexpr TypeId T_BooleanLiteral = 13893604854732947474ULL;
constexpr TypeId T_Meters = 13893604854732947475ULL;
constexpr TypeId T_Seconds = 13893604854732947476ULL;
constexpr TypeId T_LocalSlot = 13893604854732947477ULL;
constexpr TypeId T_RegionalSlot = 13893604854732947478ULL;
constexpr TypeId T_Example = 13893604854732947479ULL;
constexpr TypeId T_Other = 13893604854732947480ULL;
constexpr TypeId T_ValueSummary = 13893604854732947481ULL;
constexpr TypeId T_AddressSummary = 13893604854732947482ULL;
static constexpr Descriptor descriptors[] = {
  {T_Datum, "Datum", "org.cod.fixture.micro.Datum", true, 0, 1},
  {T_Plan, "Plan", "org.cod.fixture.micro.Plan", true, 0, 1},
  {T_LiteralPlan, "LiteralPlan", "org.cod.fixture.micro.LiteralPlan", true, 0, 1},
  {T_SourceLocated, "SourceLocated", "org.cod.fixture.micro.SourceLocated", true, 0, 1},
  {T_Scalar, "Scalar", "org.cod.fixture.micro.Scalar", true, 0, 1},
  {T_Quantity, "Quantity", "org.cod.fixture.micro.Quantity", true, 0, 1},
  {T_Destination, "Destination", "org.cod.fixture.micro.Destination", true, 0, 1},
  {T_AddressDestination, "AddressDestination", "org.cod.fixture.micro.AddressDestination", true, 0, 1},
  {T_Summary, "Summary", "org.cod.fixture.micro.Summary", true, 0, 1},
  {T_IntegerLiteral, "IntegerLiteral", "org.cod.fixture.micro.IntegerLiteral", false, 16, 8},
  {T_BooleanLiteral, "BooleanLiteral", "org.cod.fixture.micro.BooleanLiteral", false, 16, 8},
  {T_Meters, "Meters", "org.cod.fixture.micro.Meters", false, 16, 8},
  {T_Seconds, "Seconds", "org.cod.fixture.micro.Seconds", false, 16, 8},
  {T_LocalSlot, "LocalSlot", "org.cod.fixture.micro.LocalSlot", false, 16, 8},
  {T_RegionalSlot, "RegionalSlot", "org.cod.fixture.micro.RegionalSlot", false, 32, 16},
  {T_Example, "Example", "org.cod.fixture.micro.Example", false, 40, 8},
  {T_Other, "Other", "org.cod.fixture.micro.Other", false, 40, 8},
  {T_ValueSummary, "ValueSummary", "org.cod.fixture.micro.ValueSummary", false, 16, 8},
  {T_AddressSummary, "AddressSummary", "org.cod.fixture.micro.AddressSummary", false, 16, 8},
};
static constexpr TypeId root_evidence[] = {T_Example,T_Other,T_IntegerLiteral,T_BooleanLiteral,T_LocalSlot,T_RegionalSlot,T_ValueSummary,T_AddressSummary};
