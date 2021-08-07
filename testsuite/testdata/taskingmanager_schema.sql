
--
-- Name: public; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA IF NOT EXISTS public;


--
-- Name: SCHEMA public; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON SCHEMA public IS 'standard public schema';


--
-- Name: delete_duplicate_priority_geom(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION public.delete_duplicate_priority_geom() RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$
DECLARE
  proj int;
  priority int;
	bounds geometry;
BEGIN
  FOR proj, priority, bounds IN
	 SELECT project_id, priority_area_id, geometry FROM priority_duplicates
  LOOP
   DELETE  FROM public.project_priority_areas WHERE priority_area_id = priority AND project_id = proj;
   DELETE  FROM public.priority_areas WHERE id = priority ;
   INSERT INTO public.priority_areas (id, geometry) VALUES (priority, bounds);
   INSERT INTO public.project_priority_areas (project_id, priority_area_id) VALUES (proj, priority);
   RETURN NEXT proj;
  END LOOP;
END
 $$;


--
-- Name: delete_invalid_geom(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION public.delete_invalid_geom() RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$
DECLARE
   proj int;
BEGIN
   FOR proj IN
      SELECT * FROM invalid_geom
   LOOP
      DELETE FROM public.project_info WHERE project_id = proj;
      DELETE FROM public.project_chat WHERE project_id = proj;
      DELETE FROM public.task_history WHERE project_id = proj;
      DELETE FROM public.project_priority_areas WHERE project_id = proj;
      DELETE FROM public.messages WHERE project_id = proj;
      DELETE FROM public.task_invalidation_history WHERE project_id = proj;
      DELETE FROM public.tasks WHERE project_id = proj;
      DELETE FROM public.projects WHERE id = proj;
      RETURN NEXT proj;
      -- RAISE NOTICE 'Project: %', proj;
   END LOOP;
END
$$;


--
-- Name: garden_stale_projects(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION public.garden_stale_projects() RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$
DECLARE
   proj int;
BEGIN
   FOR proj IN
      SELECT * FROM stale_projects
   LOOP
      UPDATE projects SET STATUS = 0 WHERE id = proj;
      RETURN NEXT proj;
      -- RAISE NOTICE 'Project: %', proj;
   END LOOP;
END
$$;


--
-- Name: identify_invalid_geom(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION public.identify_invalid_geom() RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$
DECLARE
   task int;
   task_geometry text;
BEGIN
   FOR task IN
      SELECT * FROM invalid_geom
   LOOP
   	   RAISE NOTICE 'Task: %', task;
--  	   if task = 6134 then
-- 	   	RETURN NEXT task;
--   	   else
	     select st_asgeojson(geometry) from
	   	tasks into task_geometry where tasks.id=task and
	   	tasks.project_id=3965;
	   	RAISE NOTICE 'Geometry: %', task_geometry;
       	RETURN NEXT task;
--   end if;

   END LOOP;
END
$$;


--
-- Name: update_task_stats(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION public.update_task_stats() RETURNS SETOF text
    LANGUAGE plpgsql
    AS $$
DECLARE
   proj int;
BEGIN
   FOR proj IN
      SELECT * FROM temp_projects_id
   LOOP
      update projects set tasks_mapped = (select count(*) from tasks where project_id = proj and task_status = 2) where id = proj;
      update projects set tasks_validated = (select count(*) from tasks where project_id = proj and task_status = 4) where id = proj;
      update projects set tasks_bad_imagery = (select count(*) from tasks where project_id = proj and task_status = 6) where id = proj;
      RETURN NEXT proj;
      RAISE NOTICE 'Project: %', proj;
   END LOOP;
END
$$;


SET default_tablespace = '';

--
-- Name: alembic_version; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.alembic_version (
    version_num character varying(32) NOT NULL
);


--
-- Name: application_keys; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.application_keys (
    id integer NOT NULL,
    "user" bigint NOT NULL,
    app_key character varying NOT NULL,
    created timestamp without time zone
);


--
-- Name: application_keys_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.application_keys_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: application_keys_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.application_keys_id_seq OWNED BY public.application_keys.id;


--
-- Name: campaign_organisations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.campaign_organisations (
    campaign_id integer,
    organisation_id integer
);


--
-- Name: campaign_projects; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.campaign_projects (
    campaign_id integer,
    project_id integer
);


--
-- Name: campaigns; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.campaigns (
    id integer NOT NULL,
    name character varying NOT NULL,
    logo character varying,
    url character varying,
    description character varying
);


--
-- Name: campaigns_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.campaigns_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: campaigns_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.campaigns_id_seq OWNED BY public.campaigns.id;


--
-- Name: interests; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.interests (
    id integer NOT NULL,
    name character varying
);


--
-- Name: interests_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.interests_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: interests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.interests_id_seq OWNED BY public.interests.id;


--
-- Name: licenses; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.licenses (
    id integer NOT NULL,
    name character varying,
    description character varying,
    plain_text character varying
);


--
-- Name: licenses_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.licenses_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: licenses_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.licenses_id_seq OWNED BY public.licenses.id;


--
-- Name: mapping_issue_categories; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.mapping_issue_categories (
    id integer NOT NULL,
    name character varying NOT NULL,
    description character varying,
    archived boolean DEFAULT false NOT NULL
);


--
-- Name: mapping_issue_categories_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.mapping_issue_categories_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: mapping_issue_categories_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.mapping_issue_categories_id_seq OWNED BY public.mapping_issue_categories.id;


--
-- Name: messages; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.messages (
    id integer NOT NULL,
    message character varying,
    subject character varying,
    from_user_id bigint,
    to_user_id bigint,
    date timestamp without time zone,
    read boolean,
    message_type integer,
    project_id integer,
    task_id integer
);


--
-- Name: messages_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.messages_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: messages_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.messages_id_seq OWNED BY public.messages.id;


--
-- Name: notifications; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.notifications (
    id integer NOT NULL,
    user_id bigint,
    unread_count integer,
    date timestamp without time zone
);


--
-- Name: notifications_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.notifications_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: notifications_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.notifications_id_seq OWNED BY public.notifications.id;


--
-- Name: organisation_managers; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.organisation_managers (
    organisation_id integer NOT NULL,
    user_id bigint NOT NULL
);


--
-- Name: organisations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.organisations (
    id integer NOT NULL,
    name character varying(512) NOT NULL,
    logo character varying,
    url character varying,
    description character varying,
    type integer NOT NULL,
    slug character varying(255) NOT NULL,
    subscription_tier integer
);


--
-- Name: organisations_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.organisations_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: organisations_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.organisations_id_seq OWNED BY public.organisations.id;


--
-- Name: priority_areas; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.priority_areas (
    id integer NOT NULL,
    geometry public.geometry(Polygon,4326)
);


--
-- Name: priority_areas_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.priority_areas_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: priority_areas_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.priority_areas_id_seq OWNED BY public.priority_areas.id;


--
-- Name: project_allowed_users; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_allowed_users (
    project_id integer,
    user_id bigint
);


--
-- Name: project_chat; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_chat (
    id bigint NOT NULL,
    project_id integer NOT NULL,
    user_id integer NOT NULL,
    time_stamp timestamp without time zone NOT NULL,
    message character varying NOT NULL
);


--
-- Name: project_chat_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.project_chat_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: project_chat_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.project_chat_id_seq OWNED BY public.project_chat.id;


--
-- Name: project_custom_editors; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_custom_editors (
    project_id integer NOT NULL,
    name character varying(50) NOT NULL,
    description character varying,
    url character varying NOT NULL
);


--
-- Name: project_favorites; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_favorites (
    project_id integer,
    user_id bigint
);


--
-- Name: project_info; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_info (
    project_id integer NOT NULL,
    locale character varying(10) NOT NULL,
    name character varying(512),
    short_description character varying,
    description character varying,
    instructions character varying,
    project_id_str character varying,
    text_searchable tsvector,
    per_task_instructions character varying
);


--
-- Name: project_interests; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_interests (
    interest_id integer,
    project_id bigint
);


--
-- Name: project_priority_areas; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_priority_areas (
    project_id integer,
    priority_area_id integer
);


--
-- Name: project_teams; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.project_teams (
    team_id integer NOT NULL,
    project_id integer NOT NULL,
    role integer NOT NULL
);


--
-- Name: projects; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.projects (
    id integer NOT NULL,
    status integer NOT NULL,
    created timestamp without time zone NOT NULL,
    priority integer,
    default_locale character varying(10),
    author_id bigint NOT NULL,
    mapper_level integer NOT NULL,
    private boolean,
    changeset_comment character varying,
    due_date timestamp without time zone,
    imagery character varying,
    josm_preset character varying,
    last_updated timestamp without time zone,
    mapping_types integer[],
    total_tasks integer NOT NULL,
    tasks_mapped integer NOT NULL,
    tasks_validated integer NOT NULL,
    tasks_bad_imagery integer NOT NULL,
    license_id integer,
    centroid public.geometry(Point,4326) NOT NULL,
    geometry public.geometry(MultiPolygon,4326) NOT NULL,
    task_creation_mode integer NOT NULL,
    mapping_editors integer[] DEFAULT '{0,1,2,3}'::integer[] NOT NULL,
    validation_editors integer[] DEFAULT '{0,1,2,3}'::integer[] NOT NULL,
    osmcha_filter_id character varying,
    enforce_random_task_selection boolean,
    id_presets character varying[],
    featured boolean,
    country character varying[],
    organisation_id integer,
    mapping_permission integer,
    validation_permission integer
);


--
-- Name: projects_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.projects_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: projects_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.projects_id_seq OWNED BY public.projects.id;


--
-- Name: task_annotations; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.task_annotations (
    id integer NOT NULL,
    project_id integer,
    task_id integer NOT NULL,
    annotation_type character varying NOT NULL,
    annotation_source character varying,
    updated_timestamp timestamp without time zone NOT NULL,
    properties json NOT NULL,
    annotation_markdown character varying
);


--
-- Name: task_annotations_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.task_annotations_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: task_annotations_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.task_annotations_id_seq OWNED BY public.task_annotations.id;


--
-- Name: task_history; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.task_history (
    id integer NOT NULL,
    project_id integer,
    task_id integer NOT NULL,
    action character varying NOT NULL,
    action_text character varying,
    action_date timestamp without time zone NOT NULL,
    user_id bigint NOT NULL
);


--
-- Name: task_history_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.task_history_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: task_history_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.task_history_id_seq OWNED BY public.task_history.id;


--
-- Name: task_invalidation_history; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.task_invalidation_history (
    id integer NOT NULL,
    project_id integer NOT NULL,
    task_id integer NOT NULL,
    is_closed boolean,
    mapper_id bigint,
    mapped_date timestamp without time zone,
    invalidator_id bigint,
    invalidated_date timestamp without time zone,
    invalidation_history_id integer,
    validator_id bigint,
    validated_date timestamp without time zone,
    updated_date timestamp without time zone
);


--
-- Name: task_invalidation_history_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.task_invalidation_history_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: task_invalidation_history_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.task_invalidation_history_id_seq OWNED BY public.task_invalidation_history.id;


--
-- Name: task_mapping_issues; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.task_mapping_issues (
    id integer NOT NULL,
    mapping_issue_category_id integer NOT NULL,
    task_history_id integer NOT NULL,
    issue character varying NOT NULL,
    count integer NOT NULL
);


--
-- Name: task_mapping_issues_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.task_mapping_issues_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: task_mapping_issues_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.task_mapping_issues_id_seq OWNED BY public.task_mapping_issues.id;


--
-- Name: tasks; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.tasks (
    id integer NOT NULL,
    project_id integer NOT NULL,
    x integer,
    y integer,
    zoom integer,
    geometry public.geometry(MultiPolygon,4326),
    task_status integer,
    locked_by bigint,
    mapped_by bigint,
    validated_by bigint,
    is_square boolean,
    extra_properties character varying
);


--
-- Name: team_members; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.team_members (
    team_id integer NOT NULL,
    user_id bigint NOT NULL,
    function integer NOT NULL,
    active boolean
);


--
-- Name: teams; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.teams (
    id integer NOT NULL,
    organisation_id integer NOT NULL,
    name character varying(512) NOT NULL,
    logo character varying,
    description character varying,
    invite_only boolean NOT NULL,
    visibility integer DEFAULT 1 NOT NULL
);


--
-- Name: teams_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.teams_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: teams_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.teams_id_seq OWNED BY public.teams.id;


--
-- Name: user_interests; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.user_interests (
    interest_id integer,
    user_id bigint
);


--
-- Name: user_licenses; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.user_licenses (
    "user" bigint,
    license integer
);


--
-- Name: users; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.users (
    id bigint NOT NULL,
    username character varying,
    role integer NOT NULL,
    mapping_level integer NOT NULL,
    tasks_mapped integer NOT NULL,
    tasks_validated integer NOT NULL,
    tasks_invalidated integer NOT NULL,
    projects_mapped integer[],
    email_address character varying,
    facebook_id character varying,
    is_email_verified boolean,
    linkedin_id character varying,
    twitter_id character varying,
    date_registered timestamp without time zone,
    last_validation_date timestamp without time zone,
    tasks_notifications boolean DEFAULT true NOT NULL,
    is_expert boolean,
    irc_id character varying,
    city character varying,
    country character varying,
    name character varying,
    skype_id character varying,
    slack_id character varying,
    default_editor character varying DEFAULT 'ID'::character varying NOT NULL,
    mentions_notifications boolean DEFAULT true NOT NULL,
    comments_notifications boolean DEFAULT false NOT NULL,
    projects_notifications boolean DEFAULT true NOT NULL,
    picture_url character varying,
    gender integer,
    self_description_gender character varying,
    teams_notifications boolean DEFAULT true NOT NULL
);


--
-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.users_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;


--
-- Name: users_with_email; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.users_with_email (
    id bigint NOT NULL,
    email character varying NOT NULL
);


--
-- Name: users_with_email_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public.users_with_email_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: users_with_email_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public.users_with_email_id_seq OWNED BY public.users_with_email.id;


--
-- Name: application_keys id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.application_keys ALTER COLUMN id SET DEFAULT nextval('public.application_keys_id_seq'::regclass);


--
-- Name: campaigns id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaigns ALTER COLUMN id SET DEFAULT nextval('public.campaigns_id_seq'::regclass);


--
-- Name: interests id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.interests ALTER COLUMN id SET DEFAULT nextval('public.interests_id_seq'::regclass);


--
-- Name: licenses id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.licenses ALTER COLUMN id SET DEFAULT nextval('public.licenses_id_seq'::regclass);


--
-- Name: mapping_issue_categories id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.mapping_issue_categories ALTER COLUMN id SET DEFAULT nextval('public.mapping_issue_categories_id_seq'::regclass);


--
-- Name: messages id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages ALTER COLUMN id SET DEFAULT nextval('public.messages_id_seq'::regclass);


--
-- Name: notifications id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.notifications ALTER COLUMN id SET DEFAULT nextval('public.notifications_id_seq'::regclass);


--
-- Name: organisations id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisations ALTER COLUMN id SET DEFAULT nextval('public.organisations_id_seq'::regclass);


--
-- Name: priority_areas id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.priority_areas ALTER COLUMN id SET DEFAULT nextval('public.priority_areas_id_seq'::regclass);


--
-- Name: project_chat id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_chat ALTER COLUMN id SET DEFAULT nextval('public.project_chat_id_seq'::regclass);


--
-- Name: projects id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.projects ALTER COLUMN id SET DEFAULT nextval('public.projects_id_seq'::regclass);


--
-- Name: task_annotations id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_annotations ALTER COLUMN id SET DEFAULT nextval('public.task_annotations_id_seq'::regclass);


--
-- Name: task_history id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_history ALTER COLUMN id SET DEFAULT nextval('public.task_history_id_seq'::regclass);


--
-- Name: task_invalidation_history id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history ALTER COLUMN id SET DEFAULT nextval('public.task_invalidation_history_id_seq'::regclass);


--
-- Name: task_mapping_issues id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_mapping_issues ALTER COLUMN id SET DEFAULT nextval('public.task_mapping_issues_id_seq'::regclass);


--
-- Name: teams id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.teams ALTER COLUMN id SET DEFAULT nextval('public.teams_id_seq'::regclass);


--
-- Name: users id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);


--
-- Name: users_with_email id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users_with_email ALTER COLUMN id SET DEFAULT nextval('public.users_with_email_id_seq'::regclass);


--
-- Name: alembic_version alembic_version_pkc; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.alembic_version
    ADD CONSTRAINT alembic_version_pkc PRIMARY KEY (version_num);


--
-- Name: application_keys application_keys_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.application_keys
    ADD CONSTRAINT application_keys_pkey PRIMARY KEY (id);


--
-- Name: campaign_organisations campaign_organisation_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaign_organisations
    ADD CONSTRAINT campaign_organisation_key UNIQUE (campaign_id, organisation_id);


--
-- Name: campaigns campaigns_name_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaigns
    ADD CONSTRAINT campaigns_name_key UNIQUE (name);


--
-- Name: campaigns campaigns_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaigns
    ADD CONSTRAINT campaigns_pkey PRIMARY KEY (id);


--
-- Name: interests interests_name_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.interests
    ADD CONSTRAINT interests_name_key UNIQUE (name);


--
-- Name: interests interests_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.interests
    ADD CONSTRAINT interests_pkey PRIMARY KEY (id);


--
-- Name: licenses licenses_name_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.licenses
    ADD CONSTRAINT licenses_name_key UNIQUE (name);


--
-- Name: licenses licenses_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.licenses
    ADD CONSTRAINT licenses_pkey PRIMARY KEY (id);


--
-- Name: mapping_issue_categories mapping_issue_categories_name_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.mapping_issue_categories
    ADD CONSTRAINT mapping_issue_categories_name_key UNIQUE (name);


--
-- Name: mapping_issue_categories mapping_issue_categories_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.mapping_issue_categories
    ADD CONSTRAINT mapping_issue_categories_pkey PRIMARY KEY (id);


--
-- Name: messages messages_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_pkey PRIMARY KEY (id);


--
-- Name: notifications notifications_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.notifications
    ADD CONSTRAINT notifications_pkey PRIMARY KEY (id);


--
-- Name: organisation_managers organisation_user_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisation_managers
    ADD CONSTRAINT organisation_user_key UNIQUE (organisation_id, user_id);


--
-- Name: organisations organisations_name_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisations
    ADD CONSTRAINT organisations_name_key UNIQUE (name);


--
-- Name: organisations organisations_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisations
    ADD CONSTRAINT organisations_pkey PRIMARY KEY (id);


--
-- Name: organisations organisations_slug_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisations
    ADD CONSTRAINT organisations_slug_key UNIQUE (slug);


--
-- Name: priority_areas priority_areas_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.priority_areas
    ADD CONSTRAINT priority_areas_pkey PRIMARY KEY (id);


--
-- Name: project_chat project_chat_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_chat
    ADD CONSTRAINT project_chat_pkey PRIMARY KEY (id);


--
-- Name: project_custom_editors project_custom_editors_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_custom_editors
    ADD CONSTRAINT project_custom_editors_pkey PRIMARY KEY (project_id);


--
-- Name: project_info project_info_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_info
    ADD CONSTRAINT project_info_pkey PRIMARY KEY (project_id, locale);


--
-- Name: project_teams project_teams_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_teams
    ADD CONSTRAINT project_teams_pkey PRIMARY KEY (team_id, project_id);


--
-- Name: projects projects_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.projects
    ADD CONSTRAINT projects_pkey PRIMARY KEY (id);


--
-- Name: task_annotations task_annotations_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_annotations
    ADD CONSTRAINT task_annotations_pkey PRIMARY KEY (id);


--
-- Name: task_history task_history_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_history
    ADD CONSTRAINT task_history_pkey PRIMARY KEY (id);


--
-- Name: task_invalidation_history task_invalidation_history_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT task_invalidation_history_pkey PRIMARY KEY (id);


--
-- Name: task_mapping_issues task_mapping_issues_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_mapping_issues
    ADD CONSTRAINT task_mapping_issues_pkey PRIMARY KEY (id);


--
-- Name: tasks tasks_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.tasks
    ADD CONSTRAINT tasks_pkey PRIMARY KEY (id, project_id);


--
-- Name: team_members team_members_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.team_members
    ADD CONSTRAINT team_members_pkey PRIMARY KEY (team_id, user_id);


--
-- Name: teams teams_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.teams
    ADD CONSTRAINT teams_pkey PRIMARY KEY (id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: users users_username_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_username_key UNIQUE (username);


--
-- Name: users_with_email users_with_email_email_key; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users_with_email
    ADD CONSTRAINT users_with_email_email_key UNIQUE (email);


--
-- Name: users_with_email users_with_email_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.users_with_email
    ADD CONSTRAINT users_with_email_pkey PRIMARY KEY (id);


--
-- Name: idx_geometry; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_geometry ON public.projects USING gist (geometry);


--
-- Name: idx_project_info_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_project_info_composite ON public.project_info USING btree (locale, project_id);


--
-- Name: idx_task_annotations_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_annotations_composite ON public.task_annotations USING btree (task_id, project_id);


--
-- Name: idx_task_history_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_history_composite ON public.task_history USING btree (task_id, project_id);


--
-- Name: idx_task_history_project_id_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_history_project_id_user_id ON public.task_history USING btree (user_id, project_id);


--
-- Name: idx_task_validation_history_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_validation_history_composite ON public.task_invalidation_history USING btree (task_id, project_id);


--
-- Name: idx_task_validation_mapper_status_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_validation_mapper_status_composite ON public.task_invalidation_history USING btree (mapper_id, is_closed);


--
-- Name: idx_task_validation_validator_status_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_task_validation_validator_status_composite ON public.task_invalidation_history USING btree (invalidator_id, is_closed);


--
-- Name: idx_username_lower; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_username_lower ON public.users USING btree (lower((username)::text));


--
-- Name: ix_messages_message_type; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_messages_message_type ON public.messages USING btree (message_type);


--
-- Name: ix_messages_project_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_messages_project_id ON public.messages USING btree (project_id);


--
-- Name: ix_messages_task_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_messages_task_id ON public.messages USING btree (task_id);


--
-- Name: ix_messages_to_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_messages_to_user_id ON public.messages USING btree (to_user_id);


--
-- Name: ix_notifications_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_notifications_user_id ON public.notifications USING btree (user_id);


--
-- Name: ix_project_chat_project_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_project_chat_project_id ON public.project_chat USING btree (project_id);


--
-- Name: ix_projects_mapper_level; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_projects_mapper_level ON public.projects USING btree (mapper_level);


--
-- Name: ix_projects_mapping_editors; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_projects_mapping_editors ON public.projects USING btree (mapping_editors);


--
-- Name: ix_projects_mapping_types; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_projects_mapping_types ON public.projects USING btree (mapping_types);


--
-- Name: ix_projects_organisation_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_projects_organisation_id ON public.projects USING btree (organisation_id);


--
-- Name: ix_projects_validation_editors; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_projects_validation_editors ON public.projects USING btree (validation_editors);


--
-- Name: ix_task_annotations_project_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_task_annotations_project_id ON public.task_annotations USING btree (project_id);


--
-- Name: ix_task_history_project_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_task_history_project_id ON public.task_history USING btree (project_id);


--
-- Name: ix_task_history_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_task_history_user_id ON public.task_history USING btree (user_id);


--
-- Name: ix_task_mapping_issues_task_history_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_task_mapping_issues_task_history_id ON public.task_mapping_issues USING btree (task_history_id);


--
-- Name: ix_tasks_locked_by; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_tasks_locked_by ON public.tasks USING btree (locked_by);


--
-- Name: ix_tasks_mapped_by; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_tasks_mapped_by ON public.tasks USING btree (mapped_by);


--
-- Name: ix_tasks_project_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_tasks_project_id ON public.tasks USING btree (project_id);


--
-- Name: ix_tasks_validated_by; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_tasks_validated_by ON public.tasks USING btree (validated_by);


--
-- Name: ix_users_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_users_id ON public.users USING btree (id);


--
-- Name: ix_users_with_email_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX ix_users_with_email_id ON public.users_with_email USING btree (id);


--
-- Name: textsearch_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX textsearch_idx ON public.project_info USING gin (text_searchable);


--
-- Name: project_info tsvectorupdate; Type: TRIGGER; Schema: public; Owner: -
--

CREATE TRIGGER tsvectorupdate BEFORE INSERT OR UPDATE ON public.project_info FOR EACH ROW EXECUTE FUNCTION tsvector_update_trigger('text_searchable', 'pg_catalog.english', 'project_id_str', 'short_description', 'description');


--
-- Name: application_keys application_keys_user_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.application_keys
    ADD CONSTRAINT application_keys_user_fkey FOREIGN KEY ("user") REFERENCES public.users(id);


--
-- Name: campaign_organisations campaign_organisations_campaign_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaign_organisations
    ADD CONSTRAINT campaign_organisations_campaign_id_fkey FOREIGN KEY (campaign_id) REFERENCES public.campaigns(id);


--
-- Name: campaign_organisations campaign_organisations_organisation_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaign_organisations
    ADD CONSTRAINT campaign_organisations_organisation_id_fkey FOREIGN KEY (organisation_id) REFERENCES public.organisations(id);


--
-- Name: campaign_projects campaign_projects_campaign_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaign_projects
    ADD CONSTRAINT campaign_projects_campaign_id_fkey FOREIGN KEY (campaign_id) REFERENCES public.campaigns(id);


--
-- Name: campaign_projects campaign_projects_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.campaign_projects
    ADD CONSTRAINT campaign_projects_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: task_invalidation_history fk_invalidation_history; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT fk_invalidation_history FOREIGN KEY (invalidation_history_id) REFERENCES public.task_history(id);


--
-- Name: task_invalidation_history fk_invalidators; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT fk_invalidators FOREIGN KEY (invalidator_id) REFERENCES public.users(id);


--
-- Name: task_mapping_issues fk_issue_category; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_mapping_issues
    ADD CONSTRAINT fk_issue_category FOREIGN KEY (mapping_issue_category_id) REFERENCES public.mapping_issue_categories(id);


--
-- Name: projects fk_licenses; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.projects
    ADD CONSTRAINT fk_licenses FOREIGN KEY (license_id) REFERENCES public.licenses(id);


--
-- Name: task_invalidation_history fk_mappers; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT fk_mappers FOREIGN KEY (mapper_id) REFERENCES public.users(id);


--
-- Name: messages fk_message_projects; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT fk_message_projects FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: teams fk_organisations; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.teams
    ADD CONSTRAINT fk_organisations FOREIGN KEY (organisation_id) REFERENCES public.organisations(id);


--
-- Name: projects fk_organisations; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.projects
    ADD CONSTRAINT fk_organisations FOREIGN KEY (organisation_id) REFERENCES public.organisations(id);


--
-- Name: task_annotations fk_task_annotations; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_annotations
    ADD CONSTRAINT fk_task_annotations FOREIGN KEY (task_id, project_id) REFERENCES public.tasks(id, project_id);


--
-- Name: task_history fk_tasks; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_history
    ADD CONSTRAINT fk_tasks FOREIGN KEY (task_id, project_id) REFERENCES public.tasks(id, project_id);


--
-- Name: task_invalidation_history fk_tasks; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT fk_tasks FOREIGN KEY (task_id, project_id) REFERENCES public.tasks(id, project_id);


--
-- Name: team_members fk_teams; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.team_members
    ADD CONSTRAINT fk_teams FOREIGN KEY (team_id) REFERENCES public.teams(id);


--
-- Name: projects fk_users; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.projects
    ADD CONSTRAINT fk_users FOREIGN KEY (author_id) REFERENCES public.users(id);


--
-- Name: task_history fk_users; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_history
    ADD CONSTRAINT fk_users FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: team_members fk_users; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.team_members
    ADD CONSTRAINT fk_users FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: tasks fk_users_locked; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.tasks
    ADD CONSTRAINT fk_users_locked FOREIGN KEY (locked_by) REFERENCES public.users(id);


--
-- Name: tasks fk_users_mapper; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.tasks
    ADD CONSTRAINT fk_users_mapper FOREIGN KEY (mapped_by) REFERENCES public.users(id);


--
-- Name: tasks fk_users_validator; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.tasks
    ADD CONSTRAINT fk_users_validator FOREIGN KEY (validated_by) REFERENCES public.users(id);


--
-- Name: task_invalidation_history fk_validators; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT fk_validators FOREIGN KEY (validator_id) REFERENCES public.users(id);


--
-- Name: messages messages_from_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_from_user_id_fkey FOREIGN KEY (from_user_id) REFERENCES public.users(id);


--
-- Name: messages messages_tasks; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_tasks FOREIGN KEY (task_id, project_id) REFERENCES public.tasks(id, project_id);


--
-- Name: messages messages_to_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_to_user_id_fkey FOREIGN KEY (to_user_id) REFERENCES public.users(id);


--
-- Name: notifications notifications_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.notifications
    ADD CONSTRAINT notifications_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: organisation_managers organisation_managers_organisation_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisation_managers
    ADD CONSTRAINT organisation_managers_organisation_id_fkey FOREIGN KEY (organisation_id) REFERENCES public.organisations(id);


--
-- Name: organisation_managers organisation_managers_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.organisation_managers
    ADD CONSTRAINT organisation_managers_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: project_allowed_users project_allowed_users_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_allowed_users
    ADD CONSTRAINT project_allowed_users_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_allowed_users project_allowed_users_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_allowed_users
    ADD CONSTRAINT project_allowed_users_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: project_chat project_chat_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_chat
    ADD CONSTRAINT project_chat_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_chat project_chat_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_chat
    ADD CONSTRAINT project_chat_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: project_custom_editors project_custom_editors_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_custom_editors
    ADD CONSTRAINT project_custom_editors_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_favorites project_favorites_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_favorites
    ADD CONSTRAINT project_favorites_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_favorites project_favorites_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_favorites
    ADD CONSTRAINT project_favorites_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: project_info project_info_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_info
    ADD CONSTRAINT project_info_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_priority_areas project_priority_areas_priority_area_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_priority_areas
    ADD CONSTRAINT project_priority_areas_priority_area_id_fkey FOREIGN KEY (priority_area_id) REFERENCES public.priority_areas(id);


--
-- Name: project_priority_areas project_priority_areas_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_priority_areas
    ADD CONSTRAINT project_priority_areas_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_teams project_teams_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_teams
    ADD CONSTRAINT project_teams_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: project_teams project_teams_team_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_teams
    ADD CONSTRAINT project_teams_team_id_fkey FOREIGN KEY (team_id) REFERENCES public.teams(id);


--
-- Name: project_interests projects_interests_interest_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_interests
    ADD CONSTRAINT projects_interests_interest_id_fkey FOREIGN KEY (interest_id) REFERENCES public.interests(id);


--
-- Name: project_interests projects_interests_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.project_interests
    ADD CONSTRAINT projects_interests_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: task_annotations task_annotations_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_annotations
    ADD CONSTRAINT task_annotations_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: task_history task_history_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_history
    ADD CONSTRAINT task_history_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: task_invalidation_history task_invalidation_history_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_invalidation_history
    ADD CONSTRAINT task_invalidation_history_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: task_mapping_issues task_mapping_issues_task_history_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.task_mapping_issues
    ADD CONSTRAINT task_mapping_issues_task_history_id_fkey FOREIGN KEY (task_history_id) REFERENCES public.task_history(id);


--
-- Name: tasks tasks_project_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.tasks
    ADD CONSTRAINT tasks_project_id_fkey FOREIGN KEY (project_id) REFERENCES public.projects(id);


--
-- Name: user_interests users_interests_interest_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.user_interests
    ADD CONSTRAINT users_interests_interest_id_fkey FOREIGN KEY (interest_id) REFERENCES public.interests(id);


--
-- Name: user_interests users_interests_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.user_interests
    ADD CONSTRAINT users_interests_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: user_licenses users_licenses_license_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.user_licenses
    ADD CONSTRAINT users_licenses_license_fkey FOREIGN KEY (license) REFERENCES public.licenses(id);


--
-- Name: user_licenses users_licenses_user_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.user_licenses
    ADD CONSTRAINT users_licenses_user_fkey FOREIGN KEY ("user") REFERENCES public.users(id);


--
-- PostgreSQL database dump complete
--

