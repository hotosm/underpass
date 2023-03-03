import React from 'react';
import { ValidateOsmChange } from '../components/Underpass/ValidateOsmChange';

const osmChange = "\
<?xml version='1.0' encoding='UTF-8'?> \
<osmChange version='0.6' generator='Osmo sis 0.47.4'> \
  <create> \
    <node id='-111766' visible='true' lat='4.62042952837' lon='21.72600147299' /> \
    <node id='-111767' visible='true' lat='4.62042742837' lon='21.72608657299' /> \
    <node id='-111768' visible='true' lat='4.62036492836' lon='21.72608497299' /> \
    <node id='-111769' visible='true' lat='4.62036702836' lon='21.72599987299' /> \
    <way id='-101874' visible='true'> \
      <nd ref='-111766' /> \
      <nd ref='-111767' /> \
      <nd ref='-111768' /> \
      <nd ref='-111769' /> \
      <nd ref='-111766' /> \
      <tag k='building' v='yes'/> \
      <tag k='roof:material' v='water'/> \
    </way> \
  </create> \
</osmChange>";

export default (
    <ValidateOsmChange osmchange={osmChange} />
)

