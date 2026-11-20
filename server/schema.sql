CREATE TABLE IF NOT EXISTS scores (
  id         SERIAL PRIMARY KEY,
  name       VARCHAR(16) NOT NULL CHECK (length(trim(name)) > 0),
  score      INTEGER     NOT NULL CHECK (score >= 0),
  created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS scores_score_idx ON scores (score DESC);
