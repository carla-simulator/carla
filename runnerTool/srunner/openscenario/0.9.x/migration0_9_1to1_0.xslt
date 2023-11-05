<!--
ASAM OpenSCENARIO V1.0.0

This file allows transformation from version 0.9.1 to V 1.0.0

© by ASAM e.V., 2020
-->
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:output method='xml' indent='yes' />
  <xsl:strip-space elements='*' />
  <xsl:template name='RelativeDistanceType'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="longitudinal"'>longitudinal</xsl:when>
      <xsl:when test='$oldValue="lateral"'>lateral</xsl:when>
      <xsl:otherwise>cartesianDistance</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name='Rule'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="greater_than"'>greaterThan</xsl:when>
      <xsl:when test='$oldValue="less_than"'>lessThan</xsl:when>
      <xsl:otherwise>equalTo</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name='MiscObjectCategory'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="other"'>none</xsl:when>
      <xsl:when test='$oldValue="guardRail"'>railing</xsl:when>
      <xsl:otherwise>barrier</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name='StoryboardElementState'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="end"'>endTransition</xsl:when>
      <xsl:when test='$oldValue="cancel"'>stopTransition</xsl:when>
      <xsl:otherwise>completeState</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name='ConditionEdge'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="any"'>risingOrFalling</xsl:when>
      <xsl:when test='$oldValue="falling"'>falling</xsl:when>
      <xsl:otherwise>rising</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name='FollowingMode'>
    <xsl:param name='oldValue' />
    <xsl:choose>
      <xsl:when test='$oldValue="position"'>position</xsl:when>
      <xsl:otherwise>follow</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match='/'>
    <xsl:call-template name='unsupportedMigration' />
    <OpenSCENARIO>
      <xsl:for-each select='*'>
        <xsl:copy-of select='@*' />
        <xsl:call-template name='OpenScenario' />
      </xsl:for-each>
    </OpenSCENARIO>
  </xsl:template><xsl:template name="unsupportedMigration"/>

  <xsl:template name='OpenScenario'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="FileHeader"'>
        <FileHeader>
          <xsl:call-template name='FileHeader' />
        </FileHeader>
      </xsl:if>
      <xsl:if test='name()="ParameterDeclaration"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
      <xsl:if test='name()="Catalogs"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
      <xsl:if test='name()="RoadNetwork"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
      <xsl:if test='name()="Entities"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
      <xsl:if test='name()="Storyboard"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
      <xsl:if test='name()="Catalog"'>
        <xsl:call-template name='OpenScenarioCategory' />
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='OpenScenarioCategory'>
    <xsl:if test='name()="ParameterDeclaration"'>
      <xsl:call-template name='ScenarioDefinition' />
    </xsl:if>
    <xsl:if test='name()="Catalogs"'>
      <xsl:call-template name='ScenarioDefinition' />
    </xsl:if>
    <xsl:if test='name()="RoadNetwork"'>
      <xsl:call-template name='ScenarioDefinition' />
    </xsl:if>
    <xsl:if test='name()="Entities"'>
      <xsl:call-template name='ScenarioDefinition' />
    </xsl:if>
    <xsl:if test='name()="Storyboard"'>
      <xsl:call-template name='ScenarioDefinition' />
    </xsl:if>
    <xsl:if test='name()="Catalog"'>
      <xsl:call-template name='CatalogDefinition' />
    </xsl:if>
  </xsl:template>
  <xsl:template name='CatalogDefinition'>
    <xsl:if test='name()="Catalog"'>
      <Catalog>
        <xsl:call-template name='Catalog' />
      </Catalog>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ScenarioDefinition'>
    <xsl:if test='name()="ParameterDeclaration"'>
      <ParameterDeclarations>
        <xsl:call-template name='ParameterDeclarations' />
      </ParameterDeclarations>
    </xsl:if>
    <xsl:if test='name()="Catalogs"'>
      <CatalogLocations>
        <xsl:call-template name='CatalogLocations' />
      </CatalogLocations>
    </xsl:if>
    <xsl:if test='name()="RoadNetwork"'>
      <RoadNetwork>
        <xsl:call-template name='RoadNetwork' />
      </RoadNetwork>
    </xsl:if>
    <xsl:if test='name()="Entities"'>
      <Entities>
        <xsl:call-template name='Entities' />
      </Entities>
    </xsl:if>
    <xsl:if test='name()="Storyboard"'>
      <Storyboard>
        <xsl:call-template name='Storyboard' />
      </Storyboard>
    </xsl:if>
  </xsl:template>
  <xsl:template name='EntityObject'>
    <xsl:if test='name()="CatalogReference"'>
      <CatalogReference>
        <xsl:call-template name='CatalogReference' />
      </CatalogReference>
    </xsl:if>
    <xsl:if test='name()="Vehicle"'>
      <Vehicle>
        <xsl:call-template name='Vehicle' />
      </Vehicle>
    </xsl:if>
    <xsl:if test='name()="Pedestrian"'>
      <Pedestrian>
        <xsl:call-template name='Pedestrian' />
      </Pedestrian>
    </xsl:if>
    <xsl:if test='name()="MiscObject"'>
      <MiscObject>
        <xsl:call-template name='MiscObject' />
      </MiscObject>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ParameterDeclarations'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Parameter"'>
        <ParameterDeclaration>
          <xsl:call-template name='ParameterDeclaration' />
        </ParameterDeclaration>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='File'>
    <xsl:if test='@filepath'>
      <xsl:attribute name='filepath'>
        <xsl:value-of select='@filepath' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Directory'>
    <xsl:if test='@path'>
      <xsl:attribute name='path'>
        <xsl:value-of select='@path' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="CatalogLocations">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;VehicleCatalog&quot;">
      <VehicleCatalog>
        <xsl:call-template name="VehicleCatalogLocation"/>
      </VehicleCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;DriverCatalog&quot;">
      <ControllerCatalog>
        <xsl:call-template name="ControllerCatalogLocation"/>
      </ControllerCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;PedestrianCatalog&quot;">
      <PedestrianCatalog>
        <xsl:call-template name="PedestrianCatalogLocation"/>
      </PedestrianCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;MiscObjectCatalog&quot;">
      <MiscObjectCatalog>
        <xsl:call-template name="MiscObjectCatalogLocation"/>
      </MiscObjectCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;EnvironmentCatalog&quot;">
      <EnvironmentCatalog>
        <xsl:call-template name="EnvironmentCatalogLocation"/>
      </EnvironmentCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;ManeuverCatalog&quot;">
      <ManeuverCatalog>
        <xsl:call-template name="ManeuverCatalogLocation"/>
      </ManeuverCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;TrajectoryCatalog&quot;">
      <TrajectoryCatalog>
        <xsl:call-template name="TrajectoryCatalogLocation"/>
      </TrajectoryCatalog>
    </xsl:if>
    <xsl:if test="name()=&quot;RouteCatalog&quot;">
      <RouteCatalog>
        <xsl:call-template name="RouteCatalogLocation"/>
      </RouteCatalog>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="PedestrianControllerCatalog">
    <xsl:message terminate="no">
      WARNING: Review catalogs since driver catalog and pedestrian catalogs are merged into controller catalog
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='ConditionGroup'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Condition"'>
        <Condition>
          <xsl:call-template name='Condition' />
        </Condition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='CatalogReference'>
    <xsl:if test='@catalogName'>
      <xsl:attribute name='catalogName'>
        <xsl:value-of select='@catalogName' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@entryName'>
      <xsl:attribute name='entryName'>
        <xsl:value-of select='@entryName' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterAssignment"'>
        <ParameterAssignments>
          <xsl:call-template name='ParameterAssignments' />
        </ParameterAssignments>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterAssignments'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Parameter"'>
        <ParameterAssignment>
          <xsl:call-template name='ParameterAssignment' />
        </ParameterAssignment>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='UserDataList'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="UserData"'>
        <UserData>
          <xsl:call-template name='UserData' />
        </UserData>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='FileHeader'>
    <xsl:if test='@revMajor'>
      <xsl:attribute name='revMajor'>
        <xsl:value-of select='@revMajor' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@revMinor'>
      <xsl:attribute name='revMinor'>
        <xsl:value-of select='@revMinor' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@date'>
      <xsl:attribute name='date'>
        <xsl:value-of select='@date' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@description'>
      <xsl:attribute name='description'>
        <xsl:value-of select='@description' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@author'>
      <xsl:attribute name='author'>
        <xsl:value-of select='@author' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Pedestrian'>
    <xsl:if test='@model'>
      <xsl:attribute name='model'>
        <xsl:value-of select='@model' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@mass'>
      <xsl:attribute name='mass'>
        <xsl:value-of select='@mass' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@category'>
      <xsl:attribute name='pedestrianCategory'>
        <xsl:value-of select='@category' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="BoundingBox"'>
        <BoundingBox>
          <xsl:call-template name='BoundingBox' />
        </BoundingBox>
      </xsl:if>
      <xsl:if test='name()="Properties"'>
        <Properties>
          <xsl:call-template name='Properties' />
        </Properties>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Vehicle'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@category'>
      <xsl:attribute name='vehicleCategory'>
        <xsl:value-of select='@category' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="BoundingBox"'>
        <BoundingBox>
          <xsl:call-template name='BoundingBox' />
        </BoundingBox>
      </xsl:if>
      <xsl:if test='name()="Performance"'>
        <Performance>
          <xsl:call-template name='Performance' />
        </Performance>
      </xsl:if>
      <xsl:if test='name()="Axles"'>
        <Axles>
          <xsl:call-template name='Axles' />
        </Axles>
      </xsl:if>
      <xsl:if test='name()="Properties"'>
        <Properties>
          <xsl:call-template name='Properties' />
        </Properties>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Axle'>
    <xsl:if test='@maxSteering'>
      <xsl:attribute name='maxSteering'>
        <xsl:value-of select='@maxSteering' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@wheelDiameter'>
      <xsl:attribute name='wheelDiameter'>
        <xsl:value-of select='@wheelDiameter' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@trackWidth'>
      <xsl:attribute name='trackWidth'>
        <xsl:value-of select='@trackWidth' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@positionX'>
      <xsl:attribute name='positionX'>
        <xsl:value-of select='@positionX' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@positionZ'>
      <xsl:attribute name='positionZ'>
        <xsl:value-of select='@positionZ' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='MiscObject'>
    <xsl:if test='@category'>
      <xsl:attribute name='miscObjectCategory'>
        <xsl:call-template name='MiscObjectCategory'>
          <xsl:with-param name='oldValue' select='@category' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@mass'>
      <xsl:attribute name='mass'>
        <xsl:value-of select='@mass' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="BoundingBox"'>
        <BoundingBox>
          <xsl:call-template name='BoundingBox' />
        </BoundingBox>
      </xsl:if>
      <xsl:if test='name()="Properties"'>
        <Properties>
          <xsl:call-template name='Properties' />
        </Properties>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Condition">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@delay">
    <xsl:attribute name="delay">
      <xsl:value-of select="@delay"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@edge">
    <xsl:attribute name="conditionEdge">
      <xsl:call-template name="ConditionEdge">
        <xsl:with-param name="oldValue" select="@edge"/>
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;ByEntity&quot;">
      <ByEntityCondition>
        <xsl:call-template name="ByEntityCondition"/>
      </ByEntityCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;ByState&quot;">
      <ByValueCondition>
        <xsl:call-template name="ByStateCondition"/>
      </ByValueCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;ByValue&quot;">
      <ByValueCondition>
        <xsl:call-template name="ByValueCondition"/>
      </ByValueCondition>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="PrivateAction">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Longitudinal&quot;">
      <LongitudinalAction>
        <xsl:call-template name="LongitudinalAction"/>
      </LongitudinalAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Lateral&quot;">
      <LateralAction>
        <xsl:call-template name="LateralAction"/>
      </LateralAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Visibility&quot;">
      <VisibilityAction>
        <xsl:call-template name="VisibilityAction"/>
      </VisibilityAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Meeting&quot;">
      <MeetingAction>
        <xsl:call-template name="MeetingAction"/>
      </MeetingAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Autonomous&quot;">
      <ActivateControllerAction>
        <xsl:call-template name="ActivateControllerAction"/>
      </ActivateControllerAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Controller&quot;">
      <ControllerAction>
        <xsl:call-template name="ControllerAction"/>
      </ControllerAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Position&quot;">
      <TeleportAction>
        <Position>
          <xsl:call-template name="Position"/>
        </Position>
      </TeleportAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Routing&quot;">
      <RoutingAction>
        <xsl:call-template name="RoutingAction"/>
      </RoutingAction>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='UserDefinedAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Command"'>
        <CommandAction>
          <xsl:call-template name='CustomCommandAction' />
        </CommandAction>
      </xsl:if>
      <xsl:if test='name()="Script"'>
        <Script>
          <xsl:call-template name='ScriptAction' />
        </Script>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='GlobalAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="SetEnvironment"'>
        <EnvironmentAction>
          <xsl:call-template name='EnvironmentAction' />
        </EnvironmentAction>
      </xsl:if>
      <xsl:if test='name()="Entity"'>
        <EntityAction>
          <xsl:call-template name='EntityAction' />
        </EntityAction>
      </xsl:if>
      <xsl:if test='name()="Parameter"'>
        <ParameterAction>
          <xsl:call-template name='ParameterAction' />
        </ParameterAction>
      </xsl:if>
      <xsl:if test='name()="Infrastructure"'>
        <InfrastructureAction>
          <xsl:call-template name='InfrastructureAction' />
        </InfrastructureAction>
      </xsl:if>
      <xsl:if test='name()="Traffic"'>
        <TrafficAction>
          <xsl:call-template name='TrafficAction' />
        </TrafficAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Maneuver'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="Event"'>
        <Event>
          <xsl:call-template name='Event' />
        </Event>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TrafficDefinition'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="VehicleDistribution"'>
        <VehicleCategoryDistribution>
          <xsl:call-template name='VehicleCategoryDistribution' />
        </VehicleCategoryDistribution>
      </xsl:if>
      <xsl:if test='name()="DriverDistribution"'>
        <ControllerDistribution>
          <xsl:call-template name='ControllerDistribution' />
        </ControllerDistribution>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Environment'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="TimeOfDay"'>
        <TimeOfDay>
          <xsl:call-template name='TimeOfDay' />
        </TimeOfDay>
      </xsl:if>
      <xsl:if test='name()="Weather"'>
        <Weather>
          <xsl:call-template name='Weather' />
        </Weather>
      </xsl:if>
      <xsl:if test='name()="RoadCondition"'>
        <RoadCondition>
          <xsl:call-template name='RoadCondition' />
        </RoadCondition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Controller">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;ParameterDeclaration&quot;">
      <ParameterDeclarations>
        <xsl:call-template name="ParameterDeclarations"/>
      </ParameterDeclarations>
    </xsl:if>
    <xsl:if test="name()=&quot;Description&quot;">
      <Properties>
        <xsl:call-template name="PersonDescription"/>
      </Properties>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="PersonDescription">
  <xsl:if test="@weight">
    <Property name="weight">
      <xsl:attribute name="value">
        <xsl:value-of select="@weight"/>
      </xsl:attribute>
    </Property>
  </xsl:if>
  <xsl:if test="@height">
    <Property name="height">
      <xsl:attribute name="value">
        <xsl:value-of select="@height"/>
      </xsl:attribute>
    </Property>
  </xsl:if>
  <xsl:if test="@eyeDistance">
    <Property name="eyeDistance">
      <xsl:attribute name="value">
        <xsl:value-of select="@eyeDistance"/>
      </xsl:attribute>
    </Property>
  </xsl:if>
  <xsl:if test="@age">
    <Property name="age">
      <xsl:attribute name="value">
        <xsl:value-of select="@age"/>
      </xsl:attribute>
    </Property>
  </xsl:if>
  <xsl:if test="@sex">
    <Property name="sex">
      <xsl:attribute name="value">
        <xsl:value-of select="@sex"/>
      </xsl:attribute>
    </Property>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Properties&quot;">
      <xsl:call-template name="Properties"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='Route'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@closed'>
      <xsl:attribute name='closed'>
        <xsl:value-of select='@closed' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterDeclaration"'>
        <ParameterDeclarations>
          <xsl:call-template name='ParameterDeclarations' />
        </ParameterDeclarations>
      </xsl:if>
      <xsl:if test='name()="Waypoint"'>
        <Waypoint>
          <xsl:call-template name='Waypoint' />
        </Waypoint>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Trajectory">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@closed">
    <xsl:attribute name="closed">
      <xsl:value-of select="@closed"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;ParameterDeclaration&quot;">
      <ParameterDeclarations>
        <xsl:call-template name="ParameterDeclarations"/>
      </ParameterDeclarations>
    </xsl:if>
  </xsl:for-each>
  <Shape>
    <Polyline>
      <xsl:for-each select="*">
        <xsl:if test="name()=&quot;Vertex&quot;">
          <Vertex>
            <xsl:call-template name="Vertex"/>
          </Vertex>
        </xsl:if>
      </xsl:for-each>
    </Polyline>
  </Shape>
  <xsl:message terminate="no">
    WARNING: OSCTrajectory is restructured and connot be automatically migrated. Review XML
  </xsl:message>
</xsl:template>

  <xsl:template name='Position'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="World"'>
        <WorldPosition>
          <xsl:call-template name='WorldPosition' />
        </WorldPosition>
      </xsl:if>
      <xsl:if test='name()="RelativeWorld"'>
        <RelativeWorldPosition>
          <xsl:call-template name='RelativeWorldPosition' />
        </RelativeWorldPosition>
      </xsl:if>
      <xsl:if test='name()="RelativeObject"'>
        <RelativeObjectPosition>
          <xsl:call-template name='RelativeObjectPosition' />
        </RelativeObjectPosition>
      </xsl:if>
      <xsl:if test='name()="Road"'>
        <RoadPosition>
          <xsl:call-template name='RoadPosition' />
        </RoadPosition>
      </xsl:if>
      <xsl:if test='name()="RelativeRoad"'>
        <RelativeRoadPosition>
          <xsl:call-template name='RelativeRoadPosition' />
        </RelativeRoadPosition>
      </xsl:if>
      <xsl:if test='name()="Lane"'>
        <LanePosition>
          <xsl:call-template name='LanePosition' />
        </LanePosition>
      </xsl:if>
      <xsl:if test='name()="RelativeLane"'>
        <RelativeLanePosition>
          <xsl:call-template name='RelativeLanePosition' />
        </RelativeLanePosition>
      </xsl:if>
      <xsl:if test='name()="Route"'>
        <RoutePosition>
          <xsl:call-template name='RoutePosition' />
        </RoutePosition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='BoundingBox'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Center"'>
        <Center>
          <xsl:call-template name='Center' />
        </Center>
      </xsl:if>
      <xsl:if test='name()="Dimension"'>
        <Dimensions>
          <xsl:call-template name='Dimensions' />
        </Dimensions>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Properties'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Property"'>
        <Property>
          <xsl:call-template name='Property' />
        </Property>
      </xsl:if>
      <xsl:if test='name()="File"'>
        <File>
          <xsl:call-template name='File' />
        </File>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Orientation'>
    <xsl:if test='@type'>
      <xsl:attribute name='type'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@h'>
      <xsl:attribute name='h'>
        <xsl:value-of select='@h' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@p'>
      <xsl:attribute name='p'>
        <xsl:value-of select='@p' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@r'>
      <xsl:attribute name='r'>
        <xsl:value-of select='@r' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="Catalog">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Vehicle&quot;">
      <Vehicle>
        <xsl:call-template name="Vehicle"/>
      </Vehicle>
    </xsl:if>
    <xsl:if test="name()=&quot;Driver&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
    <xsl:if test="name()=&quot;Pedestrian&quot;">
      <Pedestrian>
        <xsl:call-template name="Pedestrian"/>
      </Pedestrian>
    </xsl:if>
    <xsl:if test="name()=&quot;PedestrianController&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
    <xsl:if test="name()=&quot;MiscObject&quot;">
      <MiscObject>
        <xsl:call-template name="MiscObject"/>
      </MiscObject>
    </xsl:if>
    <xsl:if test="name()=&quot;Environment&quot;">
      <Environment>
        <xsl:call-template name="Environment"/>
      </Environment>
    </xsl:if>
    <xsl:if test="name()=&quot;Maneuver&quot;">
      <Maneuver>
        <xsl:call-template name="Maneuver"/>
      </Maneuver>
    </xsl:if>
    <xsl:if test="name()=&quot;Trajectory&quot;">
      <Trajectory>
        <xsl:call-template name="Trajectory"/>
      </Trajectory>
    </xsl:if>
    <xsl:if test="name()=&quot;Route&quot;">
      <Route>
        <xsl:call-template name="Route"/>
      </Route>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="PedestrianController">
    <xsl:message terminate="no">
      WARNING: Review catalogs since driver catalog and pedestrian catalogs are merged into controller catalog
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='RoadNetwork'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Logics"'>
        <LogicFile>
          <xsl:call-template name='File' />
        </LogicFile>
      </xsl:if>
      <xsl:if test='name()="SceneGraph"'>
        <SceneGraphFile>
          <xsl:call-template name='File' />
        </SceneGraphFile>
      </xsl:if>
      <xsl:if test='name()="Signals"'>
        <TrafficSignals>
          <xsl:call-template name='TrafficSignals' />
        </TrafficSignals>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="TrafficSignals">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Controller&quot;">
      <TrafficSignalController>
        <xsl:call-template name="TrafficSignalController"/>
      </TrafficSignalController>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='TrafficSignalController'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@delay'>
      <xsl:attribute name='delay'>
        <xsl:value-of select='@delay' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@reference'>
      <xsl:attribute name='reference'>
        <xsl:value-of select='@reference' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Phase"'>
        <Phase>
          <xsl:call-template name='Phase' />
        </Phase>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Phase'>
    <xsl:if test='@type'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@duration'>
      <xsl:attribute name='duration'>
        <xsl:value-of select='@duration' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Signal"'>
        <TrafficSignalState>
          <xsl:call-template name='TrafficSignalState' />
        </TrafficSignalState>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TrafficSignalState'>
    <xsl:if test='@name'>
      <xsl:attribute name='trafficSignalId'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@state'>
      <xsl:attribute name='state'>
        <xsl:value-of select='@state' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Entities'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Object"'>
        <ScenarioObject>
          <xsl:call-template name='ScenarioObject' />
        </ScenarioObject>
      </xsl:if>
      <xsl:if test='name()="Selection"'>
        <EntitySelection>
          <xsl:call-template name='EntitySelection' />
        </EntitySelection>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ScenarioObject'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="CatalogReference"'>
        <xsl:call-template name='EntityObject' />
      </xsl:if>
      <xsl:if test='name()="Vehicle"'>
        <xsl:call-template name='EntityObject' />
      </xsl:if>
      <xsl:if test='name()="Pedestrian"'>
        <xsl:call-template name='EntityObject' />
      </xsl:if>
      <xsl:if test='name()="MiscObject"'>
        <xsl:call-template name='EntityObject' />
      </xsl:if>
      <xsl:if test='name()="Controller"'>
        <ObjectController>
          <xsl:call-template name='ObjectController' />
        </ObjectController>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="ObjectController">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;CatalogReference&quot;">
      <CatalogReference>
        <xsl:call-template name="CatalogReference"/>
      </CatalogReference>
    </xsl:if>
    <xsl:if test="name()=&quot;Driver&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
    <xsl:if test="name()=&quot;PedestrianController&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="PedestrianController">
    <xsl:message terminate="no">
      WARNING: Review catalogs since driver catalog and pedestrian catalogs are merged into controller catalog
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='EntitySelection'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Members"'>
        <Members>
          <xsl:call-template name='SelectedEntities' />
        </Members>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='SelectedEntities'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ByEntity"'>
        <EntityRef>
          <xsl:call-template name='EntityRef' />
        </EntityRef>
      </xsl:if>
      <xsl:if test='name()="ByType"'>
        <ByType>
          <xsl:call-template name='ByType' />
        </ByType>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EntityRef'>
    <xsl:if test='@name'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ByType'>
    <xsl:if test='@type'>
      <xsl:attribute name='objectType'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Storyboard'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Init"'>
        <Init>
          <xsl:call-template name='Init' />
        </Init>
      </xsl:if>
      <xsl:if test='name()="Story"'>
        <Story>
          <xsl:call-template name='Story' />
        </Story>
      </xsl:if>
      <xsl:if test='name()="EndConditions"'>
        <StopTrigger>
          <xsl:call-template name='Trigger' />
        </StopTrigger>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Init'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Actions"'>
        <Actions>
          <xsl:call-template name='InitActions' />
        </Actions>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="InitActions">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Global&quot;">
      <xsl:if test="not(Traffic/Jam)">
        <GlobalAction>
          <xsl:call-template name="GlobalAction"/>
        </GlobalAction>
      </xsl:if>
    </xsl:if>
    <xsl:if test="name()=&quot;UserDefined&quot;">
      <xsl:if test="not(Script)">
        <UserDefinedAction>
          <xsl:call-template name="UserDefinedAction"/>
        </UserDefinedAction>
      </xsl:if>
    </xsl:if>
    <xsl:if test="name()=&quot;Private&quot;">
      <Private>
        <xsl:call-template name="Private"/>
      </Private>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="Global/Traffic/Jam">
    <xsl:message terminate="no">
      WARNING: Jam is desupported: removed during migration
    </xsl:message>
  </xsl:if>
  <xsl:if test="UserDefined/Script">
    <xsl:message terminate="no">
      WARNING: Script is desupported: removed during migration
    </xsl:message>
  </xsl:if>
</xsl:template>
<xsl:template name="Private">
  <xsl:if test="@object">
    <xsl:attribute name="entityRef">
      <xsl:value-of select="@object"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Action&quot;">
      <xsl:if test="not(Meeting)">
        <PrivateAction>
          <xsl:call-template name="PrivateAction"/>
        </PrivateAction>
      </xsl:if>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="Action/Meeting">
    <xsl:message terminate="no">
      WARNING: OSCPrivateAction.Meeting is desupported: removed during migration
    </xsl:message>
  </xsl:if>
</xsl:template>
<xsl:template name="Story">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@owner and @owner != &quot;&quot;">
    <ParameterDeclarations>
      <ParameterDeclaration parameterType="string" name="$owner">
        <xsl:attribute name="value">
          <xsl:value-of select="@owner"/>
        </xsl:attribute>
      </ParameterDeclaration>
    </ParameterDeclarations>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Act&quot;">
      <Act>
        <xsl:call-template name="Act"/>
      </Act>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="Act">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Sequence&quot;">
      <ManeuverGroup>
        <xsl:call-template name="ManeuverGroup"/>
      </ManeuverGroup>
    </xsl:if>
    <xsl:if test="name()=&quot;Conditions&quot;">
      <xsl:for-each select="*">
        <xsl:if test="name()=&quot;Start&quot;">
          <StartTrigger>
            <xsl:call-template name="Trigger"/>
          </StartTrigger>
        </xsl:if>
        <xsl:if test="name()=&quot;End&quot;">
          <StopTrigger>
            <xsl:call-template name="Trigger"/>
          </StopTrigger>
        </xsl:if>
        <xsl:if test="name()=&quot;Cancel&quot;">
          <StopTrigger>
            <xsl:call-template name="Trigger"/>
          </StopTrigger>
        </xsl:if>
      </xsl:for-each>
    </xsl:if>
    <xsl:if test="Cancel and End">
      <xsl:message terminate="no">
        ERROR: Act: Cancel and End are both defined: Version 1.0 only supports Stop Trigger. Invalid XML is produced and needs to be revisited.
      </xsl:message>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='ManeuverGroup'>
    <xsl:if test='@numberOfExecutions'>
      <xsl:attribute name='maximumExecutionCount'>
        <xsl:value-of select='@numberOfExecutions' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Actors"'>
        <Actors>
          <xsl:call-template name='Actors' />
        </Actors>
      </xsl:if>
      <xsl:if test='name()="CatalogReference"'>
        <CatalogReference>
          <xsl:call-template name='CatalogReference' />
        </CatalogReference>
      </xsl:if>
      <xsl:if test='name()="Maneuver"'>
        <Maneuver>
          <xsl:call-template name='Maneuver' />
        </Maneuver>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Actors">
  <xsl:choose>
    <xsl:when test="ByCondition">
      <xsl:if test="ByCondition[@actor = &quot;triggeringEntity&quot;]">
        <xsl:attribute name="selectTriggeringEntities">
          <xsl:value-of select="&quot;true&quot;"/>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="ByCondition[@actor = &quot;anyEntity&quot;]">
        <xsl:attribute name="selectTriggeringEntities">
          <xsl:value-of select="&quot;false&quot;"/>
        </xsl:attribute>
        <xsl:message terminate="no">
          WARNING: Storyboard.Story.Act.Sequence.Actors.ByCondition: "anyEntity" is de-supported for version 1.0
        </xsl:message>
      </xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:attribute name="selectTriggeringEntities">
        <xsl:value-of select="&quot;false&quot;"/>
      </xsl:attribute>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Entity&quot;">
      <EntityRef>
        <xsl:call-template name="EntityRef"/>
      </EntityRef>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='SelectActorByTrigger'>
    <xsl:if test='@actor'>
      <xsl:attribute name='selectionFilter'>
        <xsl:value-of select='@actor' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ConditionSet'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Start"'>
        <StartConditions>
          <xsl:call-template name='Trigger' />
        </StartConditions>
      </xsl:if>
      <xsl:if test='name()="End"'>
        <EndConditions>
          <xsl:call-template name='Trigger' />
        </EndConditions>
      </xsl:if>
      <xsl:if test='name()="Cancel"'>
        <CancelConditions>
          <xsl:call-template name='Trigger' />
        </CancelConditions>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Trigger'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ConditionGroup"'>
        <ConditionGroup>
          <xsl:call-template name='ConditionGroup' />
        </ConditionGroup>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterDeclaration'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@type'>
      <xsl:attribute name='parameterType'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='VehicleCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ControllerCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='PedestrianCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='MiscObjectCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EnvironmentCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ManeuverCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TrajectoryCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RouteCatalogLocation'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Directory"'>
        <Directory>
          <xsl:call-template name='Directory' />
        </Directory>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterAssignment'>
    <xsl:if test='@name'>
      <xsl:attribute name='parameterRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='UserData'>
    <xsl:if test='@code'>
      <xsl:attribute name='code'>
        <xsl:value-of select='@code' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="Performance">
  <xsl:if test="@maxSpeed">
    <xsl:attribute name="maxSpeed">
      <xsl:value-of select="@maxSpeed"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@maxAcceleration">
    <xsl:attribute name="maxAcceleration">
      <xsl:value-of select="@maxAcceleration"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@maxDeceleration">
    <xsl:attribute name="maxDeceleration">
      <xsl:value-of select="@maxDeceleration"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

  <xsl:template name='Axles'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Front"'>
        <FrontAxle>
          <xsl:call-template name='Axle' />
        </FrontAxle>
      </xsl:if>
      <xsl:if test='name()="Rear"'>
        <RearAxle>
          <xsl:call-template name='Axle' />
        </RearAxle>
      </xsl:if>
      <xsl:if test='name()="Additional"'>
        <AdditionalAxle>
          <xsl:call-template name='Axle' />
        </AdditionalAxle>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ByEntityCondition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="TriggeringEntities"'>
        <TriggeringEntities>
          <xsl:call-template name='TriggeringEntities' />
        </TriggeringEntities>
      </xsl:if>
      <xsl:if test='name()="EntityCondition"'>
        <EntityCondition>
          <xsl:call-template name='EntityCondition' />
        </EntityCondition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TriggeringEntities'>
    <xsl:if test='@rule'>
      <xsl:attribute name='triggeringEntitiesRule'>
        <xsl:value-of select='@rule' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Entity"'>
        <EntityRef>
          <xsl:call-template name='EntityRef' />
        </EntityRef>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EntityCondition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="EndOfRoad"'>
        <EndOfRoadCondition>
          <xsl:call-template name='EndOfRoadCondition' />
        </EndOfRoadCondition>
      </xsl:if>
      <xsl:if test='name()="Collision"'>
        <CollisionCondition>
          <xsl:call-template name='CollisionCondition' />
        </CollisionCondition>
      </xsl:if>
      <xsl:if test='name()="Offroad"'>
        <OffroadCondition>
          <xsl:call-template name='OffroadCondition' />
        </OffroadCondition>
      </xsl:if>
      <xsl:if test='name()="TimeHeadway"'>
        <TimeHeadwayCondition>
          <xsl:call-template name='TimeHeadwayCondition' />
        </TimeHeadwayCondition>
      </xsl:if>
      <xsl:if test='name()="TimeToCollision"'>
        <TimeToCollisionCondition>
          <xsl:call-template name='TimeToCollisionCondition' />
        </TimeToCollisionCondition>
      </xsl:if>
      <xsl:if test='name()="Acceleration"'>
        <AccelerationCondition>
          <xsl:call-template name='AccelerationCondition' />
        </AccelerationCondition>
      </xsl:if>
      <xsl:if test='name()="StandStill"'>
        <StandStillCondition>
          <xsl:call-template name='StandStillCondition' />
        </StandStillCondition>
      </xsl:if>
      <xsl:if test='name()="Speed"'>
        <SpeedCondition>
          <xsl:call-template name='SpeedCondition' />
        </SpeedCondition>
      </xsl:if>
      <xsl:if test='name()="RelativeSpeed"'>
        <RelativeSpeedCondition>
          <xsl:call-template name='RelativeSpeedCondition' />
        </RelativeSpeedCondition>
      </xsl:if>
      <xsl:if test='name()="TraveledDistance"'>
        <TraveledDistanceCondition>
          <xsl:call-template name='TraveledDistanceCondition' />
        </TraveledDistanceCondition>
      </xsl:if>
      <xsl:if test='name()="ReachPosition"'>
        <ReachPositionCondition>
          <xsl:call-template name='ReachPositionCondition' />
        </ReachPositionCondition>
      </xsl:if>
      <xsl:if test='name()="Distance"'>
        <DistanceCondition>
          <xsl:call-template name='DistanceCondition' />
        </DistanceCondition>
      </xsl:if>
      <xsl:if test='name()="RelativeDistance"'>
        <RelativeDistanceCondition>
          <xsl:call-template name='RelativeDistanceCondition' />
        </RelativeDistanceCondition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EndOfRoadCondition'>
    <xsl:if test='@duration'>
      <xsl:attribute name='duration'>
        <xsl:value-of select='@duration' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='CollisionCondition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ByEntity"'>
        <EntityRef>
          <xsl:call-template name='EntityRef' />
        </EntityRef>
      </xsl:if>
      <xsl:if test='name()="ByType"'>
        <ByType>
          <xsl:call-template name='ByObjectType' />
        </ByType>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ByObjectType'>
    <xsl:if test='@type'>
      <xsl:attribute name='type'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OffroadCondition'>
    <xsl:if test='@duration'>
      <xsl:attribute name='duration'>
        <xsl:value-of select='@duration' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TimeHeadwayCondition'>
    <xsl:if test='@entity'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@entity' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@freespace'>
      <xsl:attribute name='freespace'>
        <xsl:value-of select='@freespace' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@alongRoute'>
      <xsl:attribute name='alongRoute'>
        <xsl:value-of select='@alongRoute' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TimeToCollisionCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@freespace'>
      <xsl:attribute name='freespace'>
        <xsl:value-of select='@freespace' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@alongRoute'>
      <xsl:attribute name='alongRoute'>
        <xsl:value-of select='@alongRoute' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Target"'>
        <TimeToCollisionConditionTarget>
          <xsl:call-template name='TimeToCollisionConditionTarget' />
        </TimeToCollisionConditionTarget>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TimeToCollisionConditionTarget'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
      <xsl:if test='name()="Entity"'>
        <EntityRef>
          <xsl:call-template name='EntityRef' />
        </EntityRef>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='AccelerationCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='StandStillCondition'>
    <xsl:if test='@duration'>
      <xsl:attribute name='duration'>
        <xsl:value-of select='@duration' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='SpeedCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='RelativeSpeedCondition'>
    <xsl:if test='@entity'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@entity' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TraveledDistanceCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ReachPositionCondition'>
    <xsl:if test='@tolerance'>
      <xsl:attribute name='tolerance'>
        <xsl:value-of select='@tolerance' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='DistanceCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@freespace'>
      <xsl:attribute name='freespace'>
        <xsl:value-of select='@freespace' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@alongRoute'>
      <xsl:attribute name='alongRoute'>
        <xsl:value-of select='@alongRoute' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeDistanceCondition'>
    <xsl:if test='@entity'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@entity' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@type'>
      <xsl:attribute name='relativeDistanceType'>
        <xsl:call-template name='RelativeDistanceType'>
          <xsl:with-param name='oldValue' select='@type' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@freespace'>
      <xsl:attribute name='freespace'>
        <xsl:value-of select='@freespace' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="ByStateCondition">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;AtStart&quot;">
      <StoryboardElementStateCondition>
        <xsl:if test="@type">
          <xsl:attribute name="storyboardElementType">
            <xsl:value-of select="@type"/>
          </xsl:attribute>
        </xsl:if>
        <xsl:if test="@name">
          <xsl:attribute name="storyboardElementRef">
            <xsl:value-of select="@name"/>
          </xsl:attribute>
        </xsl:if>
        <xsl:attribute name="state">
          <xsl:value-of select="'startTransition'"/>
        </xsl:attribute>
      </StoryboardElementStateCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;AfterTermination&quot;">
      <StoryboardElementStateCondition>
        <xsl:call-template name="StoryboardElementStateCondition"/>
      </StoryboardElementStateCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;Command&quot;">
      <UserDefinedValueCondition>
        <xsl:call-template name="UserDefinedValueCondition"/>
      </UserDefinedValueCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;Signal&quot;">
      <TrafficSignalCondition>
        <xsl:call-template name="TrafficSignalCondition"/>
      </TrafficSignalCondition>
    </xsl:if>
    <xsl:if test="name()=&quot;Controller&quot;">
      <TrafficSignalControllerCondition>
        <xsl:call-template name="TrafficSignalControllerCondition"/>
      </TrafficSignalControllerCondition>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='AtStartCondition'>
    <xsl:if test='@type'>
      <xsl:attribute name='type'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='StoryboardElementStateCondition'>
    <xsl:if test='@type'>
      <xsl:attribute name='storyboardElementType'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@name'>
      <xsl:attribute name='storyboardElementRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='state'>
        <xsl:call-template name='StoryboardElementState'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="UserDefinedValueCondition">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:attribute name="rule">
    <xsl:value-of select="'equalTo'"/>
  </xsl:attribute>
  <xsl:attribute name="value">
    <xsl:value-of select="'true'"/>
  </xsl:attribute>
</xsl:template>

  <xsl:template name='TrafficSignalCondition'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@state'>
      <xsl:attribute name='state'>
        <xsl:value-of select='@state' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TrafficSignalControllerCondition'>
    <xsl:if test='@name'>
      <xsl:attribute name='trafficSignalControllerRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@state'>
      <xsl:attribute name='phase'>
        <xsl:value-of select='@state' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ByValueCondition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Parameter"'>
        <ParameterCondition>
          <xsl:call-template name='ParameterCondition' />
        </ParameterCondition>
      </xsl:if>
      <xsl:if test='name()="TimeOfDay"'>
        <TimeOfDayCondition>
          <xsl:call-template name='TimeOfDayCondition' />
        </TimeOfDayCondition>
      </xsl:if>
      <xsl:if test='name()="SimulationTime"'>
        <SimulationTimeCondition>
          <xsl:call-template name='SimulationTimeCondition' />
        </SimulationTimeCondition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterCondition'>
    <xsl:if test='@name'>
      <xsl:attribute name='parameterRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="TimeOfDayCondition">
  <xsl:if test="@rule">
    <xsl:attribute name="rule">
      <xsl:call-template name="Rule">
        <xsl:with-param name="oldValue" select="@rule"/>
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
  <xsl:variable name="hour" select="Time/@hour"/>
  <xsl:variable name="min" select="Time/@min"/>
  <xsl:variable name="sec" select="Time/@sec"/>
  <xsl:variable name="day" select="Date/@day"/>
  <xsl:variable name="month" select="Date/@month"/>
  <xsl:variable name="year" select="Date/@year"/>
  <xsl:attribute name="dateTime">
    <xsl:value-of select="format-number($year,'0000')"/>
    <xsl:value-of select="format-number($month,'-00')"/>
    <xsl:value-of select="format-number($day,'-00')"/>
    <xsl:value-of select="format-number($hour,'T00')"/>
    <xsl:value-of select="format-number($min,':00')"/>
    <xsl:value-of select="format-number($sec,':00')"/>
  </xsl:attribute>
</xsl:template>

  <xsl:template name='Time'>
    <xsl:if test='@hour'>
      <xsl:attribute name='hour'>
        <xsl:value-of select='@hour' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@min'>
      <xsl:attribute name='min'>
        <xsl:value-of select='@min' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@sec'>
      <xsl:attribute name='sec'>
        <xsl:value-of select='@sec' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Date'>
    <xsl:if test='@day'>
      <xsl:attribute name='day'>
        <xsl:value-of select='@day' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@month'>
      <xsl:attribute name='month'>
        <xsl:value-of select='@month' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@year'>
      <xsl:attribute name='year'>
        <xsl:value-of select='@year' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='SimulationTimeCondition'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@rule'>
      <xsl:attribute name='rule'>
        <xsl:call-template name='Rule'>
          <xsl:with-param name='oldValue' select='@rule' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="LongitudinalAction">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Speed&quot;">
      <SpeedAction>
        <xsl:call-template name="SpeedAction"/>
      </SpeedAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Distance&quot;">
      <LongitudinalDistanceAction continuous="true">
        <xsl:call-template name="LongitudinalDistanceAction"/>
      </LongitudinalDistanceAction>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='SpeedAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Dynamics"'>
        <SpeedActionDynamics>
          <xsl:call-template name='TransitionDynamics' />
        </SpeedActionDynamics>
      </xsl:if>
      <xsl:if test='name()="Target"'>
        <SpeedActionTarget>
          <xsl:call-template name='SpeedActionTarget' />
        </SpeedActionTarget>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="TransitionDynamics">
  <xsl:if test="@shape">
    <xsl:attribute name="dynamicsShape">
      <xsl:value-of select="@shape"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@rate">
    <xsl:attribute name="value">
      <xsl:value-of select="@rate"/>
    </xsl:attribute>
    <xsl:attribute name="dynamicsDimension">
      <xsl:value-of select="&quot;rate&quot;"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@time">
    <xsl:attribute name="value">
      <xsl:value-of select="@time"/>
    </xsl:attribute>
    <xsl:attribute name="dynamicsDimension">
      <xsl:value-of select="&quot;time&quot;"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@distance">
    <xsl:attribute name="value">
      <xsl:value-of select="@distance"/>
    </xsl:attribute>
    <xsl:attribute name="dynamicsDimension">
      <xsl:value-of select="&quot;distance&quot;"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="(@distance and @time) or (@distance and @rate) or  (@time and @rate)">
    <xsl:message terminate="no">
      WARNING: OSCPrivateAction.Longitudinal.Speed or OSCPrivateAction.Lateral.LaneChange.Dynamics: Distance, time, rate must exclude each other in the original file. This results in a invalid output.
    </xsl:message>
  </xsl:if>
  <xsl:if test="not(@distance) and not(@time) and not(@rate)">
    <xsl:message terminate="no">
      WARNING: OSCPrivateAction.Longitudinal.Speed or OSCPrivateAction.Lateral.LaneChange.Dynamics: There must be at least one of time, rate or distance in the original file. This results in a invalid output.
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='SpeedActionTarget'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Relative"'>
        <RelativeTargetSpeed>
          <xsl:call-template name='RelativeTargetSpeed' />
        </RelativeTargetSpeed>
      </xsl:if>
      <xsl:if test='name()="Absolute"'>
        <AbsoluteTargetSpeed>
          <xsl:call-template name='AbsoluteTargetSpeed' />
        </AbsoluteTargetSpeed>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeTargetSpeed'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@valueType'>
      <xsl:attribute name='speedTargetValueType'>
        <xsl:value-of select='@valueType' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@continuous'>
      <xsl:attribute name='continuous'>
        <xsl:value-of select='@continuous' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='AbsoluteTargetSpeed'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="LongitudinalDistanceAction">
  <xsl:if test="@object">
    <xsl:attribute name="entityRef">
      <xsl:value-of select="@object"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@distance">
    <xsl:attribute name="distance">
      <xsl:value-of select="@distance"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@timeGap">
    <xsl:attribute name="timeGap">
      <xsl:value-of select="@timeGap"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@freespace">
    <xsl:attribute name="freespace">
      <xsl:value-of select="@freespace"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Dynamics&quot;">
      <xsl:call-template name="DynamicConstraints"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="DynamicConstraints">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Limited&quot;">
      <DynamicConstraints>
        <xsl:if test="@maxAcceleration">
          <xsl:attribute name="maxAcceleration">
            <xsl:value-of select="@maxAcceleration"/>
          </xsl:attribute>
        </xsl:if>
        <xsl:if test="@maxDeceleration">
          <xsl:attribute name="maxDeceleration">
            <xsl:value-of select="@maxDeceleration"/>
          </xsl:attribute>
        </xsl:if>
        <xsl:if test="@maxSpeed">
          <xsl:attribute name="maxSpeed">
            <xsl:value-of select="@maxSpeed"/>
          </xsl:attribute>
        </xsl:if>
      </DynamicConstraints>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='None' />
  <xsl:template name='Limited'>
    <xsl:if test='@maxAcceleration'>
      <xsl:attribute name='maxAcceleration'>
        <xsl:value-of select='@maxAcceleration' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@maxDeceleration'>
      <xsl:attribute name='maxDeceleration'>
        <xsl:value-of select='@maxDeceleration' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@maxSpeed'>
      <xsl:attribute name='maxSpeed'>
        <xsl:value-of select='@maxSpeed' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="LateralAction">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;LaneChange&quot;">
      <LaneChangeAction>
        <xsl:call-template name="LaneChangeAction"/>
      </LaneChangeAction>
    </xsl:if>
    <xsl:if test="name()=&quot;LaneOffset&quot;">
      <LaneOffsetAction continuous="true">
        <xsl:call-template name="LaneOffsetAction"/>
      </LaneOffsetAction>
    </xsl:if>
    <xsl:if test="name()=&quot;Distance&quot;">
      <LateralDistanceAction continuous="true">
        <xsl:call-template name="LateralDistanceAction"/>
      </LateralDistanceAction>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='LaneChangeAction'>
    <xsl:if test='@targetLaneOffset'>
      <xsl:attribute name='targetLaneOffset'>
        <xsl:value-of select='@targetLaneOffset' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Dynamics"'>
        <LaneChangeActionDynamics>
          <xsl:call-template name='TransitionDynamics' />
        </LaneChangeActionDynamics>
      </xsl:if>
      <xsl:if test='name()="Target"'>
        <LaneChangeTarget>
          <xsl:call-template name='LaneChangeTarget' />
        </LaneChangeTarget>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='LaneChangeTarget'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Relative"'>
        <RelativeTargetLane>
          <xsl:call-template name='RelativeTargetLane' />
        </RelativeTargetLane>
      </xsl:if>
      <xsl:if test='name()="Absolute"'>
        <AbsoluteTargetLane>
          <xsl:call-template name='AbsoluteTargetLane' />
        </AbsoluteTargetLane>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeTargetLane'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='AbsoluteTargetLane'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='LaneOffsetAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Dynamics"'>
        <LaneOffsetActionDynamics>
          <xsl:call-template name='LaneOffsetActionDynamics' />
        </LaneOffsetActionDynamics>
      </xsl:if>
      <xsl:if test='name()="Target"'>
        <LaneOffsetTarget>
          <xsl:call-template name='LaneOffsetTarget' />
        </LaneOffsetTarget>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="LaneOffsetActionDynamics">
  <xsl:if test="@maxLateralAcc">
    <xsl:attribute name="maxLateralAcc">
      <xsl:value-of select="@maxLateralAcc"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@shape">
    <xsl:attribute name="dynamicsShape">
      <xsl:value-of select="@shape"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

  <xsl:template name='LaneOffsetTarget'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Relative"'>
        <RelativeTargetLaneOffset>
          <xsl:call-template name='RelativeTargetLaneOffset' />
        </RelativeTargetLaneOffset>
      </xsl:if>
      <xsl:if test='name()="Absolute"'>
        <AbsoluteTargetLaneOffset>
          <xsl:call-template name='AbsoluteTargetLaneOffset' />
        </AbsoluteTargetLaneOffset>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeTargetLaneOffset'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='AbsoluteTargetLaneOffset'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="LateralDistanceAction">
  <xsl:if test="@object">
    <xsl:attribute name="entityRef">
      <xsl:value-of select="@object"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@distance">
    <xsl:attribute name="distance">
      <xsl:value-of select="@distance"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@freespace">
    <xsl:attribute name="freespace">
      <xsl:value-of select="@freespace"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Dynamics&quot;">
      <xsl:call-template name="DynamicConstraints"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='VisibilityAction'>
    <xsl:if test='@graphics'>
      <xsl:attribute name='graphics'>
        <xsl:value-of select='@graphics' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@traffic'>
      <xsl:attribute name='traffic'>
        <xsl:value-of select='@traffic' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@sensors'>
      <xsl:attribute name='sensors'>
        <xsl:value-of select='@sensors' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='MeetingAction'>
    <xsl:if test='@mode'>
      <xsl:attribute name='meetingPositionMode'>
        <xsl:value-of select='@mode' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@timingOffset'>
      <xsl:attribute name='timingOffset'>
        <xsl:value-of select='@timingOffset' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="ActivateControllerAction">
  <xsl:if test="@activate and @domain = &quot;lateral&quot;">
    <xsl:attribute name="lateral">
      <xsl:value-of select="@activate"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@activate and @domain = &quot;longitudinal&quot;">
    <xsl:attribute name="longitudinal">
      <xsl:value-of select="@activate"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

  <xsl:template name='ControllerAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Assign"'>
        <AssignControllerAction>
          <xsl:call-template name='AssignControllerAction' />
        </AssignControllerAction>
      </xsl:if>
      <xsl:if test='name()="Override"'>
        <OverrideControllerValueAction>
          <xsl:call-template name='OverrideControllerValueAction' />
        </OverrideControllerValueAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="AssignControllerAction">
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Driver&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
    <xsl:if test="name()=&quot;PedestrianController&quot;">
      <Controller>
        <xsl:call-template name="Controller"/>
      </Controller>
    </xsl:if>
    <xsl:if test="name()=&quot;CatalogReference&quot;">
      <CatalogReference>
        <xsl:call-template name="CatalogReference"/>
      </CatalogReference>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="PedestrianController">
    <xsl:message terminate="no">
      WARNING: Review catalogs since driver catalog and pedestrian catalogs are merged into controller catalog
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='OverrideControllerValueAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Throttle"'>
        <Throttle>
          <xsl:call-template name='OverrideThrottleAction' />
        </Throttle>
      </xsl:if>
      <xsl:if test='name()="Brake"'>
        <Brake>
          <xsl:call-template name='OverrideBrakeAction' />
        </Brake>
      </xsl:if>
      <xsl:if test='name()="Clutch"'>
        <Clutch>
          <xsl:call-template name='OverrideClutchAction' />
        </Clutch>
      </xsl:if>
      <xsl:if test='name()="ParkingBrake"'>
        <ParkingBrake>
          <xsl:call-template name='OverrideParkingBrakeAction' />
        </ParkingBrake>
      </xsl:if>
      <xsl:if test='name()="SteeringWheel"'>
        <SteeringWheel>
          <xsl:call-template name='OverrideSteeringWheelAction' />
        </SteeringWheel>
      </xsl:if>
      <xsl:if test='name()="Gear"'>
        <Gear>
          <xsl:call-template name='OverrideGearAction' />
        </Gear>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='OverrideThrottleAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OverrideBrakeAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OverrideClutchAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OverrideParkingBrakeAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OverrideSteeringWheelAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='OverrideGearAction'>
    <xsl:if test='@number'>
      <xsl:attribute name='number'>
        <xsl:value-of select='@number' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@active'>
      <xsl:attribute name='active'>
        <xsl:value-of select='@active' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='RoutingAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="FollowRoute"'>
        <AssignRouteAction>
          <xsl:call-template name='AssignRouteAction' />
        </AssignRouteAction>
      </xsl:if>
      <xsl:if test='name()="FollowTrajectory"'>
        <FollowTrajectoryAction>
          <xsl:call-template name='FollowTrajectoryAction' />
        </FollowTrajectoryAction>
      </xsl:if>
      <xsl:if test='name()="AcquirePosition"'>
        <AcquirePositionAction>
          <xsl:call-template name='AcquirePositionAction' />
        </AcquirePositionAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='AssignRouteAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Route"'>
        <Route>
          <xsl:call-template name='Route' />
        </Route>
      </xsl:if>
      <xsl:if test='name()="CatalogReference"'>
        <CatalogReference>
          <xsl:call-template name='CatalogReference' />
        </CatalogReference>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='FollowTrajectoryAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Trajectory"'>
        <Trajectory>
          <xsl:call-template name='Trajectory' />
        </Trajectory>
      </xsl:if>
      <xsl:if test='name()="CatalogReference"'>
        <CatalogReference>
          <xsl:call-template name='CatalogReference' />
        </CatalogReference>
      </xsl:if>
      <xsl:if test='name()="Longitudinal"'>
        <TimeReference>
          <xsl:call-template name='TimeReference' />
        </TimeReference>
      </xsl:if>
      <xsl:if test='name()="Lateral"'>
        <TrajectoryFollowingMode>
          <xsl:call-template name='TrajectoryFollowingMode' />
        </TrajectoryFollowingMode>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TimeReference'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="None"'>
        <None>
          <xsl:call-template name='None' />
        </None>
      </xsl:if>
      <xsl:if test='name()="Timing"'>
        <Timing>
          <xsl:call-template name='Timing' />
        </Timing>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Timing'>
    <xsl:if test='@domain'>
      <xsl:attribute name='domainAbsoluteRelative'>
        <xsl:value-of select='@domain' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@scale'>
      <xsl:attribute name='scale'>
        <xsl:value-of select='@scale' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@offset'>
      <xsl:attribute name='offset'>
        <xsl:value-of select='@offset' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TrajectoryFollowingMode'>
    <xsl:if test='@purpose'>
      <xsl:attribute name='followingMode'>
        <xsl:call-template name='FollowingMode'>
          <xsl:with-param name='oldValue' select='@purpose' />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='AcquirePositionAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='CustomCommandAction'>
    <xsl:if test='@type'>
      <xsl:attribute name='type'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ScriptAction'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@file'>
      <xsl:attribute name='file'>
        <xsl:value-of select='@file' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@execution'>
      <xsl:attribute name='execution'>
        <xsl:value-of select='@execution' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ParameterAssignment"'>
        <ParameterAssignment>
          <xsl:call-template name='ParameterAssignments' />
        </ParameterAssignment>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EnvironmentAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Environment"'>
        <Environment>
          <xsl:call-template name='Environment' />
        </Environment>
      </xsl:if>
      <xsl:if test='name()="CatalogReference"'>
        <CatalogReference>
          <xsl:call-template name='CatalogReference' />
        </CatalogReference>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='EntityAction'>
    <xsl:if test='@name'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Add"'>
        <AddEntityAction>
          <xsl:call-template name='AddEntityAction' />
        </AddEntityAction>
      </xsl:if>
      <xsl:if test='name()="Delete"'>
        <DeleteEntityAction>
          <xsl:call-template name='DeleteEntityAction' />
        </DeleteEntityAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='AddEntityAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='DeleteEntityAction' />
  <xsl:template name='ParameterAction'>
    <xsl:if test='@name'>
      <xsl:attribute name='parameterRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Set"'>
        <SetAction>
          <xsl:call-template name='ParameterSetAction' />
        </SetAction>
      </xsl:if>
      <xsl:if test='name()="Modify"'>
        <ModifyAction>
          <xsl:call-template name='ParameterModifyAction' />
        </ModifyAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterSetAction'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ParameterModifyAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Rule"'>
        <Rule>
          <xsl:call-template name='ModifyRule' />
        </Rule>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ModifyRule'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Add"'>
        <AddValue>
          <xsl:call-template name='ParameterAddValueRule' />
        </AddValue>
      </xsl:if>
      <xsl:if test='name()="Multiply"'>
        <MultiplyByValue>
          <xsl:call-template name='ParameterMultiplyByValueRule' />
        </MultiplyByValue>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ParameterAddValueRule'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ParameterMultiplyByValueRule'>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='InfrastructureAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Signal"'>
        <TrafficSignalAction>
          <xsl:call-template name='TrafficSignalAction' />
        </TrafficSignalAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TrafficSignalAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="SetController"'>
        <TrafficSignalControllerAction>
          <xsl:call-template name='TrafficSignalControllerAction' />
        </TrafficSignalControllerAction>
      </xsl:if>
      <xsl:if test='name()="SetState"'>
        <TrafficSignalStateAction>
          <xsl:call-template name='TrafficSignalStateAction' />
        </TrafficSignalStateAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='TrafficSignalControllerAction'>
    <xsl:if test='@name'>
      <xsl:attribute name='trafficSignalControllerRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@state'>
      <xsl:attribute name='phase'>
        <xsl:value-of select='@state' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TrafficSignalStateAction'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@state'>
      <xsl:attribute name='state'>
        <xsl:value-of select='@state' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TrafficAction'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Source"'>
        <TrafficSourceAction>
          <xsl:call-template name='TrafficSourceAction' />
        </TrafficSourceAction>
      </xsl:if>
      <xsl:if test='name()="Sink"'>
        <TrafficSinkAction>
          <xsl:call-template name='TrafficSinkAction' />
        </TrafficSinkAction>
      </xsl:if>
      <xsl:if test='name()="Swarm"'>
        <TrafficSwarmAction>
          <xsl:call-template name='TrafficSwarmAction' />
        </TrafficSwarmAction>
      </xsl:if>
      <xsl:if test='name()="Jam"'>
        <TrafficJamAction>
          <xsl:call-template name='TrafficJamAction' />
        </TrafficJamAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="TrafficSourceAction">
  <xsl:if test="@rate">
    <xsl:attribute name="rate">
      <xsl:value-of select="@rate"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@radius">
    <xsl:attribute name="radius">
      <xsl:value-of select="@radius"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Position&quot;">
      <Position>
        <xsl:call-template name="Position"/>
      </Position>
    </xsl:if>
    <xsl:if test="name()=&quot;TrafficDefinition&quot;">
      <TrafficDefinition>
        <xsl:call-template name="TrafficDefinition"/>
      </TrafficDefinition>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="TrafficSinkAction">
  <xsl:if test="@rate">
    <xsl:attribute name="rate">
      <xsl:value-of select="@rate"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@radius">
    <xsl:attribute name="radius">
      <xsl:value-of select="@radius"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Position&quot;">
      <Position>
        <xsl:call-template name="Position"/>
      </Position>
    </xsl:if>
    <xsl:if test="name()=&quot;TrafficDefinition&quot;">
      <TrafficDefinition>
        <xsl:call-template name="TrafficDefinition"/>
      </TrafficDefinition>
    </xsl:if>
  </xsl:for-each>
</xsl:template>
<xsl:template name="TrafficSwarmAction">
  <xsl:if test="@semiMajorAxis">
    <xsl:attribute name="semiMajorAxis">
      <xsl:value-of select="@semiMajorAxis"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@semiMinorAxis">
    <xsl:attribute name="semiMinorAxis">
      <xsl:value-of select="@semiMinorAxis"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@innerRadius">
    <xsl:attribute name="innerRadius">
      <xsl:value-of select="@innerRadius"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@offset">
    <xsl:attribute name="offset">
      <xsl:value-of select="@offset"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@numberOfVehicles">
    <xsl:attribute name="numberOfVehicles">
      <xsl:value-of select="@numberOfVehicles"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;CentralObject&quot;">
      <CentralObject>
        <xsl:call-template name="CentralSwarmObject"/>
      </CentralObject>
    </xsl:if>
    <xsl:if test="name()=&quot;TrafficDefinition&quot;">
      <TrafficDefinition>
        <xsl:call-template name="TrafficDefinition"/>
      </TrafficDefinition>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

  <xsl:template name='CentralSwarmObject'>
    <xsl:if test='@name'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='TrafficJamAction'>
    <xsl:if test='@direction'>
      <xsl:attribute name='direction'>
        <xsl:value-of select='@direction' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@speed'>
      <xsl:attribute name='speed'>
        <xsl:value-of select='@speed' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@length'>
      <xsl:attribute name='length'>
        <xsl:value-of select='@length' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
      <xsl:if test='name()="TrafficDefinition"'>
        <TrafficDefinition>
          <xsl:call-template name='TrafficDefinition' />
        </TrafficDefinition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Event">
  <xsl:if test="@name">
    <xsl:attribute name="name">
      <xsl:value-of select="@name"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@priority">
    <xsl:choose>
      <xsl:when test="@priority =&quot;following&quot;">
        <xsl:attribute name="priority">
          <xsl:value-of select="&quot;&quot;"/>
        </xsl:attribute>
        <xsl:message terminate="no">
          WARNING: Event.priority: 'following' is desupported: removed during migration. This results in unvalifd XML code.
        </xsl:message>
      </xsl:when>
      <xsl:otherwise>
        <xsl:attribute name="priority">
          <xsl:value-of select="@priority"/>
        </xsl:attribute>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Action&quot;">
      <xsl:if test="not(Global/Traffic/Jam) and not(UserDefined/Script) and not(Private/Meeting)">
        <Action>
          <xsl:call-template name="Action"/>
        </Action>
      </xsl:if>
    </xsl:if>
    <xsl:if test="name()=&quot;StartConditions&quot;">
      <StartTrigger>
        <xsl:call-template name="Trigger"/>
      </StartTrigger>
    </xsl:if>
  </xsl:for-each>
  <xsl:if test="Action/Private/Meeting">
    <xsl:message terminate="no">
      WARNING: OSCPrivateAction.Meeting  is desupported: removed during migration: This may result in unvalid XML
    </xsl:message>
  </xsl:if>
  <xsl:if test="Action/Global/Traffic/Jam">
    <xsl:message terminate="no">
      WARNING: Jam is desupported: removed during migration: This may result in unvalid XML
    </xsl:message>
  </xsl:if>
  <xsl:if test="Action/UserDefined/Script">
    <xsl:message terminate="no">
      WARNING: Script is desupported: removed during migration: This may result in unvalid XML
    </xsl:message>
  </xsl:if>
</xsl:template>

  <xsl:template name='Action'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Global"'>
        <GlobalAction>
          <xsl:call-template name='GlobalAction' />
        </GlobalAction>
      </xsl:if>
      <xsl:if test='name()="UserDefined"'>
        <UserDefinedAction>
          <xsl:call-template name='UserDefinedAction' />
        </UserDefinedAction>
      </xsl:if>
      <xsl:if test='name()="Private"'>
        <PrivateAction>
          <xsl:call-template name='PrivateAction' />
        </PrivateAction>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='VehicleCategoryDistribution'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Vehicle"'>
        <VehicleCategoryDistributionEntry>
          <xsl:call-template name='VehicleCategoryDistributionEntry' />
        </VehicleCategoryDistributionEntry>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="VehicleCategoryDistributionEntry">
  <xsl:if test="@category">
    <xsl:attribute name="category">
      <xsl:choose>
        <xsl:when test="@category='van' or @category='truck' or @category='trailer' or @category='semitrailer' or @category='bus' or @category='motorbike' or @category='bicycle' or @category='train' or @category='tram'">
          <xsl:value-of select="@category"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="'car'"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@percentage">
    <xsl:attribute name="weight">
      <xsl:value-of select="@percentage"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

  <xsl:template name='ControllerDistribution'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Driver"'>
        <ControllerDistributionEntry>
          <xsl:call-template name='ControllerDistributionEntry' />
        </ControllerDistributionEntry>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="ControllerDistributionEntry">
  <xsl:if test="@percentage">
    <xsl:attribute name="weight">
      <xsl:value-of select="@percentage"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:comment>
    Migration: Insert Controller here
  </xsl:comment>
  <Controller/>
  <xsl:message terminate="no">
    ERROR: OSCTrafficDefinition.DriverDistribution.Driver cannot be migrated automatically and will result in unvalid XML output.
  </xsl:message>
</xsl:template>
<xsl:template name="TimeOfDay">
  <xsl:if test="@animation">
    <xsl:attribute name="animation">
      <xsl:value-of select="@animation"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:variable name="hour" select="Time/@hour"/>
  <xsl:variable name="min" select="Time/@min"/>
  <xsl:variable name="sec" select="Time/@sec"/>
  <xsl:variable name="day" select="Date/@day"/>
  <xsl:variable name="month" select="Date/@month"/>
  <xsl:variable name="year" select="Date/@year"/>
  <xsl:attribute name="dateTime">
    <xsl:value-of select="format-number($year,'0000')"/>
    <xsl:value-of select="format-number($month,'-00')"/>
    <xsl:value-of select="format-number($day,'-00')"/>
    <xsl:value-of select="format-number($hour,'T00')"/>
    <xsl:value-of select="format-number($min,':00')"/>
    <xsl:value-of select="format-number($sec,':00')"/>
  </xsl:attribute>
</xsl:template>

  <xsl:template name='Weather'>
    <xsl:if test='@cloudState'>
      <xsl:attribute name='cloudState'>
        <xsl:value-of select='@cloudState' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Sun"'>
        <Sun>
          <xsl:call-template name='Sun' />
        </Sun>
      </xsl:if>
      <xsl:if test='name()="Fog"'>
        <Fog>
          <xsl:call-template name='Fog' />
        </Fog>
      </xsl:if>
      <xsl:if test='name()="Precipitation"'>
        <Precipitation>
          <xsl:call-template name='Precipitation' />
        </Precipitation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Sun'>
    <xsl:if test='@intensity'>
      <xsl:attribute name='intensity'>
        <xsl:value-of select='@intensity' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@azimuth'>
      <xsl:attribute name='azimuth'>
        <xsl:value-of select='@azimuth' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@elevation'>
      <xsl:attribute name='elevation'>
        <xsl:value-of select='@elevation' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Fog'>
    <xsl:if test='@visualRange'>
      <xsl:attribute name='visualRange'>
        <xsl:value-of select='@visualRange' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="BoundingBox"'>
        <BoundingBox>
          <xsl:call-template name='BoundingBox' />
        </BoundingBox>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Precipitation'>
    <xsl:if test='@type'>
      <xsl:attribute name='precipitationType'>
        <xsl:value-of select='@type' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@intensity'>
      <xsl:attribute name='intensity'>
        <xsl:value-of select='@intensity' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template><xsl:template name="RoadCondition">
  <xsl:if test="@frictionScale">
    <xsl:attribute name="frictionScaleFactor">
      <xsl:value-of select="@frictionScale"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

  <xsl:template name='SurfaceState'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@intensity'>
      <xsl:attribute name='intensity'>
        <xsl:value-of select='@intensity' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Waypoint'>
    <xsl:if test='@strategy'>
      <xsl:attribute name='routeStrategy'>
        <xsl:value-of select='@strategy' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Position"'>
        <Position>
          <xsl:call-template name='Position' />
        </Position>
      </xsl:if>
    </xsl:for-each>
  </xsl:template><xsl:template name="Vertex">
  <xsl:if test="@reference">
    <xsl:attribute name="time">
      <xsl:value-of select="@reference"/>
    </xsl:attribute>
  </xsl:if>
  <xsl:for-each select="*">
    <xsl:if test="name()=&quot;Position&quot;">
      <Position>
        <xsl:call-template name="Position"/>
      </Position>
    </xsl:if>
  </xsl:for-each>
  <xsl:message terminate="no">
    WARNING: OSCTrajectory.Vertex: OSCTrajectory is restructured and connot be automatically migrated. Review XML.
  </xsl:message>
</xsl:template>

  <xsl:template name='Shape'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Polyline"'>
        <Polyline>
          <xsl:call-template name='Polyline' />
        </Polyline>
      </xsl:if>
      <xsl:if test='name()="Clothoid"'>
        <Clothoid>
          <xsl:call-template name='Clothoid' />
        </Clothoid>
      </xsl:if>
      <xsl:if test='name()="Spline"'>
        <Spline>
          <xsl:call-template name='Spline' />
        </Spline>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='Polyline' />
  <xsl:template name='Clothoid'>
    <xsl:if test='@curvature'>
      <xsl:attribute name='curvature'>
        <xsl:value-of select='@curvature' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@curvatureDot'>
      <xsl:attribute name='curvatureDot'>
        <xsl:value-of select='@curvatureDot' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@length'>
      <xsl:attribute name='length'>
        <xsl:value-of select='@length' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Spline'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="ControlPoint1"'>
        <ControlPoint1>
          <xsl:call-template name='ControlPoint1' />
        </ControlPoint1>
      </xsl:if>
      <xsl:if test='name()="ControlPoint2"'>
        <ControlPoint2>
          <xsl:call-template name='ControlPoint2' />
        </ControlPoint2>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='ControlPoint1'>
    <xsl:if test='@status'>
      <xsl:attribute name='status'>
        <xsl:value-of select='@status' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='ControlPoint2'>
    <xsl:if test='@status'>
      <xsl:attribute name='status'>
        <xsl:value-of select='@status' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='WorldPosition'>
    <xsl:if test='@x'>
      <xsl:attribute name='x'>
        <xsl:value-of select='@x' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@y'>
      <xsl:attribute name='y'>
        <xsl:value-of select='@y' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@z'>
      <xsl:attribute name='z'>
        <xsl:value-of select='@z' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@h'>
      <xsl:attribute name='h'>
        <xsl:value-of select='@h' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@p'>
      <xsl:attribute name='p'>
        <xsl:value-of select='@p' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@r'>
      <xsl:attribute name='r'>
        <xsl:value-of select='@r' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='RelativeWorldPosition'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dx'>
      <xsl:attribute name='dx'>
        <xsl:value-of select='@dx' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dy'>
      <xsl:attribute name='dy'>
        <xsl:value-of select='@dy' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dz'>
      <xsl:attribute name='dz'>
        <xsl:value-of select='@dz' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeObjectPosition'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dx'>
      <xsl:attribute name='dx'>
        <xsl:value-of select='@dx' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dy'>
      <xsl:attribute name='dy'>
        <xsl:value-of select='@dy' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dz'>
      <xsl:attribute name='dz'>
        <xsl:value-of select='@dz' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RoadPosition'>
    <xsl:if test='@roadId'>
      <xsl:attribute name='roadId'>
        <xsl:value-of select='@roadId' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@s'>
      <xsl:attribute name='s'>
        <xsl:value-of select='@s' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@t'>
      <xsl:attribute name='t'>
        <xsl:value-of select='@t' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeRoadPosition'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@ds'>
      <xsl:attribute name='ds'>
        <xsl:value-of select='@ds' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dt'>
      <xsl:attribute name='dt'>
        <xsl:value-of select='@dt' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='LanePosition'>
    <xsl:if test='@roadId'>
      <xsl:attribute name='roadId'>
        <xsl:value-of select='@roadId' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@laneId'>
      <xsl:attribute name='laneId'>
        <xsl:value-of select='@laneId' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@offset'>
      <xsl:attribute name='offset'>
        <xsl:value-of select='@offset' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@s'>
      <xsl:attribute name='s'>
        <xsl:value-of select='@s' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RelativeLanePosition'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@dLane'>
      <xsl:attribute name='dLane'>
        <xsl:value-of select='@dLane' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@ds'>
      <xsl:attribute name='ds'>
        <xsl:value-of select='@ds' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@offset'>
      <xsl:attribute name='offset'>
        <xsl:value-of select='@offset' />
      </xsl:attribute>
    </xsl:if>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RoutePosition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="RouteRef"'>
        <RouteRef>
          <xsl:call-template name='RouteRef' />
        </RouteRef>
      </xsl:if>
      <xsl:if test='name()="Orientation"'>
        <Orientation>
          <xsl:call-template name='Orientation' />
        </Orientation>
      </xsl:if>
      <xsl:if test='name()="Position"'>
        <InRoutePosition>
          <xsl:call-template name='InRoutePosition' />
        </InRoutePosition>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='RouteRef'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Route"'>
        <Route>
          <xsl:call-template name='Route' />
        </Route>
      </xsl:if>
      <xsl:if test='name()="CatalogReference"'>
        <CatalogReference>
          <xsl:call-template name='CatalogReference' />
        </CatalogReference>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='InRoutePosition'>
    <xsl:for-each select='*'>
      <xsl:if test='name()="Current"'>
        <FromCurrentEntity>
          <xsl:call-template name='PositionOfCurrentEntity' />
        </FromCurrentEntity>
      </xsl:if>
      <xsl:if test='name()="RoadCoord"'>
        <FromRoadCoordinates>
          <xsl:call-template name='PositionInRoadCoordinates' />
        </FromRoadCoordinates>
      </xsl:if>
      <xsl:if test='name()="LaneCoord"'>
        <FromLaneCoordinates>
          <xsl:call-template name='PositionInLaneCoordinates' />
        </FromLaneCoordinates>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
  <xsl:template name='PositionOfCurrentEntity'>
    <xsl:if test='@object'>
      <xsl:attribute name='entityRef'>
        <xsl:value-of select='@object' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='PositionInRoadCoordinates'>
    <xsl:if test='@pathS'>
      <xsl:attribute name='pathS'>
        <xsl:value-of select='@pathS' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@t'>
      <xsl:attribute name='t'>
        <xsl:value-of select='@t' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='PositionInLaneCoordinates'>
    <xsl:if test='@pathS'>
      <xsl:attribute name='pathS'>
        <xsl:value-of select='@pathS' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@laneId'>
      <xsl:attribute name='laneId'>
        <xsl:value-of select='@laneId' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@laneOffset'>
      <xsl:attribute name='laneOffset'>
        <xsl:value-of select='@laneOffset' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Center'>
    <xsl:if test='@x'>
      <xsl:attribute name='x'>
        <xsl:value-of select='@x' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@y'>
      <xsl:attribute name='y'>
        <xsl:value-of select='@y' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@z'>
      <xsl:attribute name='z'>
        <xsl:value-of select='@z' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Dimensions'>
    <xsl:if test='@width'>
      <xsl:attribute name='width'>
        <xsl:value-of select='@width' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@length'>
      <xsl:attribute name='length'>
        <xsl:value-of select='@length' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@height'>
      <xsl:attribute name='height'>
        <xsl:value-of select='@height' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
  <xsl:template name='Property'>
    <xsl:if test='@name'>
      <xsl:attribute name='name'>
        <xsl:value-of select='@name' />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test='@value'>
      <xsl:attribute name='value'>
        <xsl:value-of select='@value' />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>